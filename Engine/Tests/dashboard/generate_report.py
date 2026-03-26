#!/usr/bin/env python3
"""
generate_report.py — ExplorerLens Test Dashboard Report Generator
Parses CTest XML output and unit test stdout into an HTML dashboard.
Sprint 31 — Test Dashboard + Visual Regression (v15.5.0)
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Sequence

PROJECT_ROOT = Path(__file__).resolve().parents[3]


# ── Data model ────────────────────────────────────────────────────────────────

@dataclass
class TestResult:
    name: str
    passed: bool
    duration_ms: float = 0.0
    message: str = ""
    suite: str = "unknown"


@dataclass
class BenchmarkResult:
    name: str
    value: float
    unit: str
    baseline: float = 0.0

    @property
    def delta_pct(self) -> float:
        if self.baseline <= 0:
            return 0.0
        return (self.value - self.baseline) / self.baseline * 100.0

    @property
    def regressed(self) -> bool:
        return self.delta_pct > 15.0


@dataclass
class ReportSummary:
    total: int = 0
    passed: int = 0
    failed: int = 0
    skipped: int = 0
    duration_ms: float = 0.0
    timestamp: str = field(
        default_factory=lambda: datetime.now(timezone.utc).isoformat()
    )
    version: str = "unknown"
    build_id: str = ""
    platform: str = ""

    @property
    def pass_rate(self) -> float:
        if self.total == 0:
            return 0.0
        return self.passed / self.total * 100.0

    @property
    def status(self) -> str:
        if self.failed > 0:
            return "FAILED"
        if self.passed == self.total and self.total > 0:
            return "PASSED"
        return "UNKNOWN"


# ── Parsers ───────────────────────────────────────────────────────────────────

def parse_ctest_xml(xml_path: Path) -> list[TestResult]:
    """Parse CTest JUnit XML output into TestResult list."""
    results: list[TestResult] = []
    if not xml_path.exists():
        return results
    try:
        tree = ET.parse(xml_path)  # noqa: S314 — local file only
        root = tree.getroot()
        for suite in root.iter("testsuite"):
            suite_name = suite.get("name", "unknown")
            for case in suite.iter("testcase"):
                name = case.get("name", "unnamed")
                time_s = float(case.get("time", "0") or "0")
                failure_el = case.find("failure")
                error_el = case.find("error")
                skipped_el = case.find("skipped")
                if skipped_el is not None:
                    continue  # count separately if needed
                passed = failure_el is None and error_el is None
                message = ""
                if failure_el is not None:
                    message = failure_el.get("message", "")
                elif error_el is not None:
                    message = error_el.get("message", "")
                results.append(TestResult(
                    name=name,
                    passed=passed,
                    duration_ms=time_s * 1000.0,
                    message=message,
                    suite=suite_name,
                ))
    except ET.ParseError as exc:
        print(f"[warn] XML parse error in {xml_path}: {exc}", file=sys.stderr)
    return results


def parse_engine_test_stdout(log_path: Path) -> tuple[ReportSummary, list[TestResult]]:
    """
    Parse EngineTests.exe stdout log.
    Expected format per line: 'PASS TestName' or 'FAIL TestName: message'
    Summary line: 'Results: N passed, M failed, K total'
    """
    summary = ReportSummary()
    results: list[TestResult] = []

    if not log_path.exists():
        return summary, results

    pass_re = re.compile(r"^PASS\s+(\S+)")
    fail_re = re.compile(r"^FAIL\s+(\S+)(?::\s*(.*))?$")
    summary_re = re.compile(
        r"Results:\s*(\d+)\s*passed,\s*(\d+)\s*failed,\s*(\d+)\s*total"
    )
    version_re = re.compile(r"ExplorerLens v([\d.]+)")

    for line in log_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = line.strip()
        m = pass_re.match(line)
        if m:
            results.append(TestResult(name=m.group(1), passed=True, suite="Engine"))
            continue
        m = fail_re.match(line)
        if m:
            results.append(TestResult(
                name=m.group(1),
                passed=False,
                message=m.group(2) or "",
                suite="Engine",
            ))
            continue
        m = summary_re.search(line)
        if m:
            summary.passed = int(m.group(1))
            summary.failed = int(m.group(2))
            summary.total = int(m.group(3))
            continue
        m = version_re.search(line)
        if m:
            summary.version = m.group(1)

    if not results:
        # Fallback: derive from summary counts
        summary.total = summary.passed + summary.failed
    else:
        summary.total = len(results)
        summary.passed = sum(1 for r in results if r.passed)
        summary.failed = sum(1 for r in results if not r.passed)

    return summary, results


def parse_benchmark_json(bench_path: Path) -> list[BenchmarkResult]:
    """Parse Google Benchmark JSON output."""
    if not bench_path.exists():
        return []
    try:
        data = json.loads(bench_path.read_text(encoding="utf-8"))
        benchmarks: list[BenchmarkResult] = []
        for entry in data.get("benchmarks", []):
            benchmarks.append(BenchmarkResult(
                name=entry.get("name", "unknown"),
                value=float(entry.get("real_time", 0)),
                unit=entry.get("time_unit", "ns"),
            ))
        return benchmarks
    except (json.JSONDecodeError, KeyError, TypeError) as exc:
        print(f"[warn] Benchmark JSON parse error: {exc}", file=sys.stderr)
        return []


# ── HTML report generator ─────────────────────────────────────────────────────

def _status_badge(status: str) -> str:
    colours = {"PASSED": "#28a745", "FAILED": "#dc3545", "UNKNOWN": "#6c757d"}
    colour = colours.get(status, "#6c757d")
    return f'<span style="background:{colour};color:#fff;padding:2px 8px;border-radius:4px;font-weight:bold">{status}</span>'


def _pct_bar(value: float, max_val: float = 100.0, colour: str = "#28a745") -> str:
    width = min(int(value / max_val * 200), 200)
    return (
        f'<div style="background:#eee;width:200px;height:12px;display:inline-block;border-radius:3px">'
        f'<div style="background:{colour};width:{width}px;height:12px;border-radius:3px"></div></div>'
        f' {value:.1f}%'
    )


def render_html(
    summary: ReportSummary,
    results: list[TestResult],
    benchmarks: list[BenchmarkResult],
) -> str:
    """Render a self-contained HTML dashboard."""
    version = PROJECT_ROOT / "VERSION"
    ver_str = version.read_text(encoding="utf-8").strip() if version.exists() else summary.version

    failed_tests = [r for r in results if not r.passed]
    passed_tests = [r for r in results if r.passed]

    failed_rows = "\n".join(
        f'<tr><td style="color:#dc3545">✗</td><td>{r.name}</td><td>{r.suite}</td>'
        f'<td style="color:#dc3545">{r.message or "—"}</td></tr>'
        for r in failed_tests
    )

    bench_rows = "\n".join(
        f'<tr><td>{b.name}</td><td>{b.value:.2f} {b.unit}</td>'
        f'<td style="color:{"#dc3545" if b.regressed else "#28a745"}">'
        f'{b.delta_pct:+.1f}%</td></tr>'
        for b in benchmarks
    ) if benchmarks else '<tr><td colspan="3" style="color:#888">No benchmark data</td></tr>'

    pass_colour = "#28a745" if summary.failed == 0 else "#dc3545"
    pct_bar = _pct_bar(summary.pass_rate, colour=pass_colour)

    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ExplorerLens v{ver_str} — Test Dashboard</title>
<style>
  body {{ font-family: Segoe UI, Arial, sans-serif; margin: 0; background: #f5f5f5; color: #333; }}
  .header {{ background: #1a1a2e; color: #fff; padding: 24px 32px; }}
  .header h1 {{ margin: 0 0 4px; font-size: 22px; }}
  .header .sub {{ opacity: 0.7; font-size: 13px; }}
  .card {{ background: #fff; border-radius: 8px; margin: 16px 32px; padding: 20px 24px;
           box-shadow: 0 1px 4px rgba(0,0,0,0.1); }}
  .stat-grid {{ display: flex; gap: 24px; flex-wrap: wrap; }}
  .stat {{ text-align: center; min-width: 100px; }}
  .stat .val {{ font-size: 36px; font-weight: 700; color: #1a1a2e; }}
  .stat .lbl {{ font-size: 12px; color: #888; text-transform: uppercase; letter-spacing: 0.5px; }}
  table {{ width: 100%; border-collapse: collapse; font-size: 13px; }}
  th {{ text-align: left; padding: 8px 12px; background: #f8f9fa; border-bottom: 2px solid #dee2e6; }}
  td {{ padding: 6px 12px; border-bottom: 1px solid #eee; }}
  tr:hover td {{ background: #f8f9fa; }}
  h2 {{ font-size: 16px; margin: 0 0 16px; color: #1a1a2e; }}
</style>
</head>
<body>
<div class="header">
  <h1>ExplorerLens v{ver_str} — Test Dashboard</h1>
  <div class="sub">Generated: {timestamp} | Build: {summary.build_id or "local"} | Status: {_status_badge(summary.status)}</div>
</div>

<div class="card">
  <h2>Summary</h2>
  <div class="stat-grid">
    <div class="stat"><div class="val" style="color:{pass_colour}">{summary.passed}</div><div class="lbl">Passed</div></div>
    <div class="stat"><div class="val" style="color:#dc3545">{summary.failed}</div><div class="lbl">Failed</div></div>
    <div class="stat"><div class="val">{summary.total}</div><div class="lbl">Total</div></div>
    <div class="stat"><div class="val">{summary.pass_rate:.1f}%</div><div class="lbl">Pass Rate</div></div>
    <div class="stat"><div class="val">{summary.duration_ms/1000:.1f}s</div><div class="lbl">Duration</div></div>
  </div>
  <div style="margin-top:16px">{pct_bar}</div>
</div>

{"" if not failed_tests else f'''<div class="card">
  <h2 style="color:#dc3545">Failed Tests ({len(failed_tests)})</h2>
  <table>
    <thead><tr><th></th><th>Test</th><th>Suite</th><th>Message</th></tr></thead>
    <tbody>{failed_rows}</tbody>
  </table>
</div>'''}

<div class="card">
  <h2>Performance Benchmarks</h2>
  <table>
    <thead><tr><th>Benchmark</th><th>Time</th><th>vs Baseline</th></tr></thead>
    <tbody>{bench_rows}</tbody>
  </table>
</div>

<div class="card" style="font-size:12px; color:#888">
  ExplorerLens Test Dashboard | Sprint 31 | v{ver_str}
</div>
</body>
</html>"""


# ── CLI entry point ───────────────────────────────────────────────────────────

def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Generate ExplorerLens HTML test dashboard",
    )
    parser.add_argument(
        "--test-log",
        type=Path,
        default=PROJECT_ROOT / "build-logs" / "test-latest.log",
        help="Path to EngineTests.exe stdout log",
    )
    parser.add_argument(
        "--ctest-xml",
        type=Path,
        default=PROJECT_ROOT / "build" / "Testing" / "Temporary" / "LastTest.log",
        help="Path to CTest XML output",
    )
    parser.add_argument(
        "--bench-json",
        type=Path,
        default=PROJECT_ROOT / "build" / "bench.json",
        help="Path to Google Benchmark JSON output",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=PROJECT_ROOT / "build-logs" / "test-dashboard.html",
        help="Output HTML path",
    )
    parser.add_argument(
        "--build-id",
        default=os.environ.get("GITHUB_RUN_ID", ""),
        help="CI build identifier",
    )
    args = parser.parse_args(argv)

    summary, results = parse_engine_test_stdout(args.test_log)
    summary.build_id = args.build_id
    summary.platform = sys.platform

    xml_results = parse_ctest_xml(args.ctest_xml)
    results.extend(xml_results)

    if not results and summary.total == 0:
        # No data — emit minimal dashboard
        summary.total = 0
        print("[warn] No test data found. Generating empty dashboard.", file=sys.stderr)

    benchmarks = parse_benchmark_json(args.bench_json)

    html = render_html(summary, results, benchmarks)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(html, encoding="utf-8")
    print(f"[dashboard] Report written → {args.out}")
    print(f"[dashboard] {summary.passed}/{summary.total} tests passed ({summary.pass_rate:.1f}%) — {summary.status}")

    return 0 if summary.status != "FAILED" else 1


if __name__ == "__main__":
    sys.exit(main())
