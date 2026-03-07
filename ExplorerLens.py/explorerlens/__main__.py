# ExplorerLens.py — CLI Entry Point
# Copyright (c) 2026 ExplorerLens Project
"""
Usage:
    python -m explorerlens                     Launch GUI manager
    python -m explorerlens --register          Register COM shell extension
    python -m explorerlens --unregister        Unregister COM shell extension
    python -m explorerlens --thumbnail FILE    Generate thumbnail for FILE
    python -m explorerlens --benchmark DIR     Run performance benchmark on DIR
    python -m explorerlens --admin             Force admin elevation
"""

import argparse
import sys
import os
import logging
from pathlib import Path


def is_admin() -> bool:
    """Check if running with administrator privileges."""
    try:
        import ctypes
        return ctypes.windll.shell32.IsUserAnAdmin() != 0
    except Exception:
        return False


def elevate_and_relaunch(args: list[str] | None = None) -> None:
    """Re-launch the current script with admin privileges via UAC prompt."""
    import ctypes
    python = sys.executable
    script = os.path.abspath(sys.argv[0])
    params = f'"{script}"'
    if args:
        params += " " + " ".join(f'"{a}"' for a in args)
    ctypes.windll.shell32.ShellExecuteW(
        None, "runas", python, params, None, 1
    )
    sys.exit(0)


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="explorerlens",
        description="ExplorerLens.py — Python Thumbnail Provider for Windows",
    )
    parser.add_argument("--register", action="store_true",
                        help="Register COM shell extension (requires admin)")
    parser.add_argument("--unregister", action="store_true",
                        help="Unregister COM shell extension (requires admin)")
    parser.add_argument("--thumbnail", metavar="FILE",
                        help="Generate thumbnail for a file")
    parser.add_argument("--size", type=int, default=256,
                        help="Thumbnail size in pixels (default: 256)")
    parser.add_argument("--output", metavar="FILE",
                        help="Output path for thumbnail (default: display)")
    parser.add_argument("--benchmark", metavar="DIR", nargs="?", const=".",
                        help="Run performance benchmark on directory")
    parser.add_argument("--nogui", action="store_true",
                        help="CLI mode only, don't launch GUI")
    parser.add_argument("--admin", action="store_true",
                        help="Force admin elevation")
    parser.add_argument("--diagnostics", metavar="FILE", nargs="?",
                        const="explorerlens-diagnostics.json",
                        help="Export diagnostic report to FILE")
    parser.add_argument("--log-level", default="INFO",
                        choices=["DEBUG", "INFO", "WARNING", "ERROR"],
                        help="Logging level (default: INFO)")

    args = parser.parse_args()

    # Setup logging
    logging.basicConfig(
        level=getattr(logging, args.log_level),
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        datefmt="%H:%M:%S",
    )

    # Admin elevation for operations that need it
    if args.admin or args.register or args.unregister:
        if not is_admin():
            elevate_and_relaunch(sys.argv[1:])
            return

    if args.diagnostics:
        from explorerlens.shell.diagnostics import export_diagnostics
        out = export_diagnostics(Path(args.diagnostics))
        print(f"Diagnostics exported to {out}")
        return

    if args.register:
        from explorerlens.shell.com_server import register
        from explorerlens.config import Config
        config = Config.load()
        ok = register(config.get_enabled_extensions())
        if ok:
            print("COM shell extension registered successfully.")
        else:
            print("Registration failed.", file=sys.stderr)
            sys.exit(1)
        return

    if args.unregister:
        from explorerlens.shell.com_server import unregister
        ok = unregister()
        if ok:
            print("COM shell extension unregistered successfully.")
        else:
            print("Unregistration failed.", file=sys.stderr)
            sys.exit(1)
        return

    if args.thumbnail:
        from explorerlens.engine import ThumbnailEngine, ThumbnailRequest, DecodeStatus
        from explorerlens.config import Config
        config = Config.load()
        engine = ThumbnailEngine(config)
        req = ThumbnailRequest(path=Path(args.thumbnail), size=args.size)
        result = engine.generate(req)
        if result.status == DecodeStatus.Success and result.image:
            if args.output:
                result.image.save(args.output)
                print(f"Thumbnail saved to {args.output}")
            else:
                result.image.show()
        else:
            print(f"Failed to generate thumbnail for {args.thumbnail}: "
                  f"{result.error}", file=sys.stderr)
            sys.exit(1)
        engine.shutdown()
        return

    if args.benchmark is not None:
        from explorerlens.utils.benchmark import run_benchmark, print_benchmark_report
        from explorerlens.config import ALL_EXTENSIONS

        target_dir = Path(args.benchmark)
        if not target_dir.is_dir():
            print(f"Not a directory: {target_dir}", file=sys.stderr)
            sys.exit(1)

        files = [
            f for f in target_dir.rglob("*")
            if f.is_file() and f.suffix.lower() in ALL_EXTENSIONS
        ]
        if not files:
            print(f"No supported files found in {target_dir}")
            return

        print(f"Benchmarking {len(files)} files from {target_dir}...")
        results = run_benchmark(files, size=args.size)
        print_benchmark_report(results)
        return

    # Default: launch GUI
    if not args.nogui:
        from explorerlens.gui.app import ExplorerLensApp
        from explorerlens.config import Config
        config = Config.load()
        app = ExplorerLensApp(config)
        app.run()


if __name__ == "__main__":
    main()
