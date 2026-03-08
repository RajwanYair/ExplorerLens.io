# ExplorerLens.py — GUI Application
# Copyright (c) 2026 ExplorerLens Project
"""
Full-featured tkinter GUI mirroring LENSManager.exe from ExplorerLens.io.
Includes:
  - Per-category format toggles (35+ checkboxes)
  - Dark / Light mode with system detection
  - Performance dashboard (cache stats, decode speed)
  - Register / Unregister buttons (with UAC elevation)
  - Config import / export
  - System tray icon support
"""

from __future__ import annotations

import ctypes
import logging
import sys
import threading
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk
from typing import Optional

from ..config import (
    CONFIG_FILE,
    FORMAT_CATEGORIES,
    FORMAT_GROUPS,
    FORMAT_REGISTRY,
    Config,
)

logger = logging.getLogger("explorerlens.gui")


class ExplorerLensApp:
    """Main GUI window — mirrors LENSManager."""

    def __init__(self, config: Config | None = None) -> None:
        self._config = config or Config.load()
        self._root: tk.Tk | None = None
        self._format_vars: dict[str, tk.BooleanVar] = {}
        self._group_all_vars: dict[str, tk.BooleanVar] = {}
        self._category_vars: dict[str, tk.BooleanVar] = {}  # legacy compat
        self._dark_mode = self._detect_dark_mode()
        self._engine = None
        self._tray_icon = None
        self._snapshot_before: dict[str, bool] | None = None

    def run(self) -> None:
        """Launch the GUI."""
        self._root = tk.Tk()
        self._root.title("ExplorerLens.py Manager")
        self._root.geometry("700x600")
        self._root.minsize(600, 500)

        self._apply_theme()
        self._build_ui()

        # Dark mode: style tkinter popup widgets that don't use ttk
        if self._dark_mode:
            self._root.option_add("*TCombobox*Listbox.background", "#2d2d30")
            self._root.option_add("*TCombobox*Listbox.foreground", "#d4d4d4")
            self._root.option_add("*TCombobox*Listbox.selectBackground", "#0e639c")
            self._root.option_add("*TCombobox*Listbox.selectForeground", "#ffffff")
            # Ensure all native tk widgets (Text, Canvas, Toplevel) get dark colors
            self._root.option_add("*Text.background", "#2d2d30")
            self._root.option_add("*Text.foreground", "#d4d4d4")
            self._root.option_add("*Text.insertBackground", "#d4d4d4")
            self._root.option_add("*Canvas.background", "#1e1e1e")
            self._root.option_add("*Listbox.background", "#2d2d30")
            self._root.option_add("*Listbox.foreground", "#d4d4d4")
            self._root.option_add("*Menu.background", "#2d2d30")
            self._root.option_add("*Menu.foreground", "#d4d4d4")
            self._root.option_add("*Menu.activeBackground", "#0e639c")
            self._root.option_add("*Menu.activeForeground", "#ffffff")
            self._root.option_add("*Toplevel.background", "#1e1e1e")
            self._root.option_add("*Label.background", "#1e1e1e")
            self._root.option_add("*Label.foreground", "#d4d4d4")
            self._root.option_add("*Frame.background", "#1e1e1e")
            self._root.option_add("*Button.background", "#0e639c")
            self._root.option_add("*Button.foreground", "#ffffff")
            self._root.option_add("*Checkbutton.background", "#1e1e1e")
            self._root.option_add("*Checkbutton.foreground", "#d4d4d4")
            self._root.option_add("*Radiobutton.background", "#1e1e1e")
            self._root.option_add("*Radiobutton.foreground", "#d4d4d4")
            self._root.option_add("*Entry.background", "#2d2d30")
            self._root.option_add("*Entry.foreground", "#d4d4d4")
            self._root.option_add("*Spinbox.background", "#2d2d30")
            self._root.option_add("*Spinbox.foreground", "#d4d4d4")

        self._root.protocol("WM_DELETE_WINDOW", self._on_close)

        if self._config.show_tray_icon:
            self._setup_tray_icon()

        self._root.mainloop()

    # ── Theme ────────────────────────────────────────────────────────

    def _detect_dark_mode(self) -> bool:
        """Detect system dark mode preference."""
        if self._config.dark_mode == "dark":
            return True
        if self._config.dark_mode == "light":
            return False
        # Auto-detect from Windows registry
        try:
            import winreg

            key = winreg.OpenKey(
                winreg.HKEY_CURRENT_USER,
                r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize",
            )
            val, _ = winreg.QueryValueEx(key, "AppsUseLightTheme")
            key.Close()
            return val == 0
        except Exception:
            return False

    def _apply_theme(self) -> None:
        """Apply dark or light theme."""
        style = ttk.Style()
        style.theme_use("clam")
        if self._dark_mode:
            bg = "#1e1e1e"
            fg = "#d4d4d4"
            field_bg = "#2d2d30"
            border = "#3e3e42"
            accent = "#0e639c"
            tab_bg = "#2d2d30"
            tab_sel = "#1e1e1e"

            self._root.configure(bg=bg)
            style.configure(
                ".",
                background=bg,
                foreground=fg,
                fieldbackground=field_bg,
                bordercolor=border,
                insertcolor=fg,
                selectbackground=accent,
                selectforeground="#ffffff",
            )
            style.configure("TFrame", background=bg)
            style.configure("TLabel", background=bg, foreground=fg)
            style.configure("TCheckbutton", background=bg, foreground=fg)
            style.map(
                "TCheckbutton",
                background=[("active", "#2a2d2e"), ("disabled", bg)],
                foreground=[("disabled", "#6a6a6a")],
            )
            style.configure("TButton", background=accent, foreground="#ffffff")
            style.map(
                "TButton",
                background=[("active", "#1177bb"), ("disabled", "#3e3e42")],
                foreground=[("disabled", "#6a6a6a")],
            )
            style.configure("TLabelframe", background=bg, foreground="#569cd6")
            style.configure("TLabelframe.Label", background=bg, foreground="#569cd6")
            style.configure("TRadiobutton", background=bg, foreground=fg)
            style.map(
                "TRadiobutton",
                background=[("active", "#2a2d2e"), ("disabled", bg)],
                foreground=[("disabled", "#6a6a6a")],
            )
            style.configure(
                "TProgressbar",
                background=accent,
                troughcolor=field_bg,
                bordercolor=border,
            )
            style.configure(
                "Treeview",
                background=field_bg,
                foreground=fg,
                fieldbackground=field_bg,
                bordercolor=border,
            )
            style.map(
                "Treeview",
                background=[("selected", accent)],
                foreground=[("selected", "#ffffff")],
            )
            # Notebook tabs — critical for dark mode text visibility
            style.configure(
                "TNotebook",
                background=bg,
                bordercolor=border,
                tabmargins=[2, 5, 2, 0],
            )
            style.configure(
                "TNotebook.Tab",
                background=tab_bg,
                foreground=fg,
                padding=[10, 4],
                bordercolor=border,
            )
            style.map(
                "TNotebook.Tab",
                background=[("selected", tab_sel), ("active", "#3e3e42")],
                foreground=[("selected", "#ffffff"), ("active", "#ffffff")],
            )
            # Combobox / Spinbox
            style.configure(
                "TCombobox",
                fieldbackground=field_bg,
                background=field_bg,
                foreground=fg,
                arrowcolor=fg,
                bordercolor=border,
                selectbackground=accent,
                selectforeground="#ffffff",
            )
            style.map(
                "TCombobox",
                fieldbackground=[("readonly", field_bg), ("disabled", bg)],
                foreground=[("readonly", fg), ("disabled", "#6a6a6a")],
            )
            style.configure(
                "TSpinbox",
                fieldbackground=field_bg,
                foreground=fg,
                arrowcolor=fg,
                bordercolor=border,
            )
            # Scrollbar
            style.configure(
                "TScrollbar",
                background="#3e3e42",
                troughcolor=bg,
                arrowcolor=fg,
                bordercolor=border,
            )
            # Entry
            style.configure(
                "TEntry",
                fieldbackground=field_bg,
                foreground=fg,
                insertcolor=fg,
                bordercolor=border,
            )
            style.configure(
                "Header.TLabel",
                font=("Segoe UI", 14, "bold"),
                foreground="#569cd6",
                background=bg,
            )
            style.configure("Status.TLabel", foreground="#608b4e", background=bg)
        else:
            style.configure(
                "Header.TLabel", font=("Segoe UI", 14, "bold"), foreground="#0066b8"
            )
            style.configure("Status.TLabel", foreground="#008000")

    # ── UI Construction ──────────────────────────────────────────────

    def _build_ui(self) -> None:
        """Build the main UI layout."""
        notebook = ttk.Notebook(self._root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        # Tab 1: Format Categories
        tab_formats = ttk.Frame(notebook)
        notebook.add(tab_formats, text="  Formats  ")
        self._build_formats_tab(tab_formats)

        # Tab 2: Performance
        tab_perf = ttk.Frame(notebook)
        notebook.add(tab_perf, text="  Performance  ")
        self._build_performance_tab(tab_perf)

        # Tab 3: Registration
        tab_reg = ttk.Frame(notebook)
        notebook.add(tab_reg, text="  Registration  ")
        self._build_registration_tab(tab_reg)

        # Tab 4: Settings
        tab_settings = ttk.Frame(notebook)
        notebook.add(tab_settings, text="  Settings  ")
        self._build_settings_tab(tab_settings)

        # Tab 5: About
        tab_about = ttk.Frame(notebook)
        notebook.add(tab_about, text="  About  ")
        self._build_about_tab(tab_about)

        # Status bar
        self._status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(
            self._root,
            textvariable=self._status_var,
            style="Status.TLabel",
            anchor=tk.W,
        )
        status_bar.pack(fill=tk.X, padx=8, pady=(0, 4))

    def _build_formats_tab(self, parent: ttk.Frame) -> None:
        """Build per-format checkboxes grouped by category (mirrors LENSManager)."""
        header = ttk.Label(
            parent, text="Supported Format Handlers", style="Header.TLabel"
        )
        header.pack(anchor=tk.W, padx=12, pady=(12, 4))

        # Toolbar: Select All / Deselect All / Save
        toolbar = ttk.Frame(parent)
        toolbar.pack(fill=tk.X, padx=12, pady=(0, 4))
        ttk.Button(toolbar, text="Select All", command=self._enable_all).pack(
            side=tk.LEFT, padx=4
        )
        ttk.Button(toolbar, text="Deselect All", command=self._disable_all).pack(
            side=tk.LEFT, padx=4
        )
        ttk.Button(toolbar, text="Apply && Save", command=self._apply_formats).pack(
            side=tk.RIGHT, padx=4
        )

        # Scrollable area
        canvas = tk.Canvas(parent, highlightthickness=0)
        scrollbar = ttk.Scrollbar(parent, orient=tk.VERTICAL, command=canvas.yview)
        scroll_frame = ttk.Frame(canvas)

        scroll_frame.bind(
            "<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        canvas.create_window((0, 0), window=scroll_frame, anchor=tk.NW)
        canvas.configure(yscrollcommand=scrollbar.set)

        if self._dark_mode:
            canvas.configure(bg="#1e1e1e")

        # Build per-group LabelFrames with per-format checkboxes
        group_row = 0
        for group_key, group_label in FORMAT_GROUPS.items():
            # Collect formats in this group
            group_formats = [
                (k, v) for k, v in FORMAT_REGISTRY.items() if v["group"] == group_key
            ]
            if not group_formats:
                continue

            group_frame = ttk.LabelFrame(scroll_frame, text=group_label)
            group_frame.grid(row=group_row, column=0, sticky=tk.EW, padx=8, pady=4)
            scroll_frame.columnconfigure(0, weight=1)

            # Per-category "All" checkbox (mirrors IDC_CB_ALL_* in EXE)
            all_var = tk.BooleanVar(value=True)
            self._group_all_vars[group_key] = all_var
            all_cb = ttk.Checkbutton(
                group_frame,
                text="All",
                variable=all_var,
                command=lambda gk=group_key: self._on_group_all_toggle(gk),
            )
            all_cb.grid(row=0, column=0, sticky=tk.W, padx=(8, 4), pady=2)

            # Per-format checkboxes within the group
            col = 1
            row_in_group = 0
            max_cols = 4
            for fmt_key, fmt_info in group_formats:
                var = tk.BooleanVar(
                    value=self._config.enabled_formats.get(fmt_key, True)
                )
                self._format_vars[fmt_key] = var

                ext_str = ", ".join(fmt_info["exts"][:3])
                cb_text = f"{fmt_info['name']} ({ext_str})"
                cb = ttk.Checkbutton(
                    group_frame,
                    text=cb_text,
                    variable=var,
                    command=lambda gk=group_key: self._on_format_toggle(gk),
                )
                cb.grid(row=row_in_group, column=col, sticky=tk.W, padx=4, pady=1)
                # Tooltip via binding
                tip_text = fmt_info["tip"]
                cb.bind("<Enter>", lambda e, t=tip_text: self._status_var.set(t))
                cb.bind("<Leave>", lambda e: self._status_var.set("Ready"))

                col += 1
                if col > max_cols:
                    col = 1
                    row_in_group += 1

            # Update the "All" checkbox initial state
            self._update_group_all_state(group_key)
            group_row += 1

        # Options section (mirrors EXE's Sort + Show Icon)
        opt_frame = ttk.LabelFrame(scroll_frame, text="Options")
        opt_frame.grid(row=group_row, column=0, sticky=tk.EW, padx=8, pady=4)

        self._sort_var = tk.BooleanVar(value=self._config.sort_thumbnails)
        ttk.Checkbutton(
            opt_frame, text="Sort thumbnails alphabetically", variable=self._sort_var
        ).grid(row=0, column=0, sticky=tk.W, padx=8, pady=2)

        self._icon_var = tk.BooleanVar(value=self._config.show_archive_icon)
        ttk.Checkbutton(
            opt_frame, text="Show archive icon overlay", variable=self._icon_var
        ).grid(row=0, column=1, sticky=tk.W, padx=8, pady=2)
        group_row += 1

        # Collage Mode section (mirrors EXE's radio buttons)
        collage_frame = ttk.LabelFrame(scroll_frame, text="Collage Mode")
        collage_frame.grid(row=group_row, column=0, sticky=tk.EW, padx=8, pady=4)

        self._collage_var = tk.IntVar(value=self._config.collage_mode)
        for i, (val, label) in enumerate(
            [
                (1, "Single Page (1×1)"),
                (4, "2×2 Grid"),
                (9, "3×3 Grid"),
                (16, "4×4 Grid"),
            ]
        ):
            ttk.Radiobutton(
                collage_frame, text=label, variable=self._collage_var, value=val
            ).grid(row=0, column=i, sticky=tk.W, padx=8, pady=4)

        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=4)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Enable mouse wheel scrolling
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")

        canvas.bind_all("<MouseWheel>", _on_mousewheel)

    def _build_performance_tab(self, parent: ttk.Frame) -> None:
        """Build performance dashboard."""
        header = ttk.Label(parent, text="Performance Dashboard", style="Header.TLabel")
        header.pack(anchor=tk.W, padx=12, pady=(12, 8))

        frame = ttk.LabelFrame(parent, text="Cache Statistics")
        frame.pack(fill=tk.X, padx=12, pady=4)

        self._cache_labels: dict[str, ttk.Label] = {}
        for i, (label, key) in enumerate(
            [
                ("Memory items:", "mem_items"),
                ("Memory usage:", "mem_mb"),
                ("Disk items:", "disk_items"),
                ("Disk usage:", "disk_mb"),
                ("Hit rate:", "hit_rate"),
            ]
        ):
            ttk.Label(frame, text=label).grid(
                row=i, column=0, sticky=tk.W, padx=8, pady=2
            )
            val_lbl = ttk.Label(frame, text="—")
            val_lbl.grid(row=i, column=1, sticky=tk.W, padx=8, pady=2)
            self._cache_labels[key] = val_lbl

        perf_frame = ttk.LabelFrame(parent, text="Engine Performance")
        perf_frame.pack(fill=tk.X, padx=12, pady=4)

        self._perf_labels: dict[str, ttk.Label] = {}
        for i, (label, key) in enumerate(
            [
                ("Total decoded:", "total"),
                ("Success:", "succeeded"),
                ("Failed:", "failed"),
                ("Avg time:", "avg_ms"),
                ("Throughput:", "img_per_sec"),
            ]
        ):
            ttk.Label(perf_frame, text=label).grid(
                row=i, column=0, sticky=tk.W, padx=8, pady=2
            )
            val_lbl = ttk.Label(perf_frame, text="—")
            val_lbl.grid(row=i, column=1, sticky=tk.W, padx=8, pady=2)
            self._perf_labels[key] = val_lbl

        ttk.Button(parent, text="Refresh Stats", command=self._refresh_stats).pack(
            padx=12, pady=8
        )

        ttk.Button(parent, text="Clear Cache", command=self._clear_cache).pack(
            padx=12, pady=4
        )

    def _build_registration_tab(self, parent: ttk.Frame) -> None:
        """Build registration controls."""
        header = ttk.Label(
            parent, text="Shell Extension Registration", style="Header.TLabel"
        )
        header.pack(anchor=tk.W, padx=12, pady=(12, 8))

        info = ttk.Label(
            parent,
            text="Register ExplorerLens.py as the Windows thumbnail "
            "provider for enabled formats.\n"
            "Requires administrator privileges.",
            wraplength=600,
        )
        info.pack(anchor=tk.W, padx=12, pady=4)

        self._admin_label = ttk.Label(parent, text="")
        self._admin_label.pack(anchor=tk.W, padx=12, pady=4)
        self._update_admin_status()

        btn_frame = ttk.Frame(parent)
        btn_frame.pack(fill=tk.X, padx=12, pady=8)

        ttk.Button(
            btn_frame, text="Register (requires Admin)", command=self._register
        ).pack(side=tk.LEFT, padx=4)
        ttk.Button(btn_frame, text="Unregister", command=self._unregister).pack(
            side=tk.LEFT, padx=4
        )
        ttk.Button(btn_frame, text="Run as Admin", command=self._elevate).pack(
            side=tk.RIGHT, padx=4
        )

        # Status list
        self._reg_status_text = tk.Text(parent, height=15, width=70, state=tk.DISABLED)
        if self._dark_mode:
            self._reg_status_text.configure(
                bg="#2d2d30", fg="#d4d4d4", insertbackground="#d4d4d4"
            )
        self._reg_status_text.pack(fill=tk.BOTH, expand=True, padx=12, pady=4)

        ttk.Button(parent, text="Check Status", command=self._check_registration).pack(
            padx=12, pady=4
        )

        # Extra tools row
        tools_frame = ttk.Frame(parent)
        tools_frame.pack(fill=tk.X, padx=12, pady=4)
        ttk.Button(
            tools_frame, text="Export Diagnostics", command=self._export_diagnostics
        ).pack(side=tk.LEFT, padx=4)
        ttk.Button(
            tools_frame, text="Detect Conflicts", command=self._detect_conflicts
        ).pack(side=tk.LEFT, padx=4)
        ttk.Button(
            tools_frame, text="Flush Explorer Cache", command=self._flush_cache
        ).pack(side=tk.LEFT, padx=4)

    def _build_settings_tab(self, parent: ttk.Frame) -> None:
        """Build settings controls."""
        header = ttk.Label(parent, text="Settings", style="Header.TLabel")
        header.pack(anchor=tk.W, padx=12, pady=(12, 8))

        frame = ttk.LabelFrame(parent, text="General")
        frame.pack(fill=tk.X, padx=12, pady=4)

        # Thumbnail size
        ttk.Label(frame, text="Thumbnail size:").grid(
            row=0, column=0, sticky=tk.W, padx=8, pady=4
        )
        self._size_var = tk.StringVar(value=str(self._config.thumbnail_size))
        size_combo = ttk.Combobox(
            frame,
            textvariable=self._size_var,
            values=["64", "128", "256", "512", "1024"],
            width=8,
        )
        size_combo.grid(row=0, column=1, sticky=tk.W, padx=8, pady=4)

        # Dark mode
        ttk.Label(frame, text="Theme:").grid(
            row=1, column=0, sticky=tk.W, padx=8, pady=4
        )
        self._theme_var = tk.StringVar(value=self._config.dark_mode)
        theme_combo = ttk.Combobox(
            frame,
            textvariable=self._theme_var,
            values=["auto", "dark", "light"],
            width=8,
        )
        theme_combo.grid(row=1, column=1, sticky=tk.W, padx=8, pady=4)

        # Max workers
        ttk.Label(frame, text="Worker threads:").grid(
            row=2, column=0, sticky=tk.W, padx=8, pady=4
        )
        self._workers_var = tk.StringVar(
            value=str(self._config.performance.max_workers)
        )
        ttk.Spinbox(
            frame, from_=1, to=16, textvariable=self._workers_var, width=8
        ).grid(row=2, column=1, sticky=tk.W, padx=8, pady=4)

        # Log level
        ttk.Label(frame, text="Log level:").grid(
            row=3, column=0, sticky=tk.W, padx=8, pady=4
        )
        self._log_var = tk.StringVar(value=self._config.log_level)
        ttk.Combobox(
            frame,
            textvariable=self._log_var,
            values=["DEBUG", "INFO", "WARNING", "ERROR"],
            width=8,
        ).grid(row=3, column=1, sticky=tk.W, padx=8, pady=4)

        # Config import / export
        io_frame = ttk.LabelFrame(parent, text="Configuration")
        io_frame.pack(fill=tk.X, padx=12, pady=8)

        ttk.Button(io_frame, text="Export Config", command=self._export_config).pack(
            side=tk.LEFT, padx=8, pady=8
        )
        ttk.Button(io_frame, text="Import Config", command=self._import_config).pack(
            side=tk.LEFT, padx=8, pady=8
        )
        ttk.Button(io_frame, text="Reset Defaults", command=self._reset_defaults).pack(
            side=tk.LEFT, padx=8, pady=8
        )

        ttk.Button(parent, text="Save Settings", command=self._save_settings).pack(
            padx=12, pady=8
        )

    def _build_about_tab(self, parent: ttk.Frame) -> None:
        """Build About tab (mirrors EXE's About dialog)."""
        header = ttk.Label(parent, text="ExplorerLens.py", style="Header.TLabel")
        header.pack(anchor=tk.W, padx=12, pady=(12, 4))

        info_lines = [
            "Python companion for ExplorerLens — Windows Shell Thumbnail Provider",
            "",
            "ExplorerLens.io  — C++/COM Shell Extension (LENSShell.dll)",
            "ExplorerLens.py  — Python GUI Manager + Decode Engine",
            "",
            f"Supported formats: {len(FORMAT_REGISTRY)}",
            f"Format groups: {len(FORMAT_GROUPS)}",
            f"COM CLSID: {{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}}",
            "",
            "Copyright (c) 2026 ExplorerLens Project",
        ]
        for line in info_lines:
            ttk.Label(parent, text=line, wraplength=600).pack(
                anchor=tk.W, padx=16, pady=1
            )

        # System info (mirrors EXE's IDC_ABOUT_SYSINFO)
        sys_frame = ttk.LabelFrame(parent, text="System Information")
        sys_frame.pack(fill=tk.X, padx=12, pady=8)

        import platform

        sys_info = [
            ("Python", platform.python_version()),
            ("Platform", platform.platform()),
            ("Architecture", platform.machine()),
        ]
        for i, (label, value) in enumerate(sys_info):
            ttk.Label(sys_frame, text=f"{label}:").grid(
                row=i, column=0, sticky=tk.W, padx=8, pady=2
            )
            ttk.Label(sys_frame, text=value).grid(
                row=i, column=1, sticky=tk.W, padx=8, pady=2
            )

    # ── Format event handlers ──────────────────────────────────────

    def _on_format_toggle(self, group_key: str) -> None:
        """Called when any individual format checkbox changes."""
        self._update_group_all_state(group_key)

    def _on_group_all_toggle(self, group_key: str) -> None:
        """Toggle all formats in a group (mirrors OnCategoryAllClicked in EXE)."""
        target = self._group_all_vars[group_key].get()
        for fmt_key, fmt_info in FORMAT_REGISTRY.items():
            if fmt_info["group"] == group_key and fmt_key in self._format_vars:
                self._format_vars[fmt_key].set(target)

    def _update_group_all_state(self, group_key: str) -> None:
        """Sync the group 'All' checkbox with individual format states."""
        states = [
            self._format_vars[k].get()
            for k, v in FORMAT_REGISTRY.items()
            if v["group"] == group_key and k in self._format_vars
        ]
        if all(states):
            self._group_all_vars[group_key].set(True)
        else:
            self._group_all_vars[group_key].set(False)

    def _on_category_toggle(self) -> None:
        for cat, var in self._category_vars.items():
            self._config.enabled_categories[cat] = var.get()

    def _enable_all(self) -> None:
        for var in self._format_vars.values():
            var.set(True)
        for var in self._group_all_vars.values():
            var.set(True)

    def _disable_all(self) -> None:
        for var in self._format_vars.values():
            var.set(False)
        for var in self._group_all_vars.values():
            var.set(False)

    def _capture_snapshot(self) -> dict[str, bool]:
        """Capture current format state for change summary."""
        return {k: v.get() for k, v in self._format_vars.items()}

    def _apply_formats(self) -> None:
        """Apply format changes with change summary (mirrors LENSManager's
        CChangeSummaryDlg)."""
        old_state = {
            k: self._config.enabled_formats.get(k, True) for k in FORMAT_REGISTRY
        }
        new_state = {k: v.get() for k, v in self._format_vars.items()}

        # Build change list
        changes: list[str] = []
        for k in FORMAT_REGISTRY:
            old_v = old_state.get(k, True)
            new_v = new_state.get(k, True)
            if old_v != new_v:
                name = FORMAT_REGISTRY[k]["name"]
                action = "ENABLED" if new_v else "DISABLED"
                changes.append(f"  {name}: {action}")

        # Options changes
        if self._sort_var.get() != self._config.sort_thumbnails:
            changes.append(f"  Sort: {'ON' if self._sort_var.get() else 'OFF'}")
        if self._icon_var.get() != self._config.show_archive_icon:
            changes.append(f"  Icon overlay: {'ON' if self._icon_var.get() else 'OFF'}")
        if self._collage_var.get() != self._config.collage_mode:
            changes.append(f"  Collage mode: {self._collage_var.get()}")

        if not changes:
            self._status_var.set("No changes to apply")
            return

        summary = f"{len(changes)} change(s):\n" + "\n".join(changes)
        if messagebox.askyesno("Apply Changes", f"Apply these changes?\n\n{summary}"):
            self._config.enabled_formats = new_state
            self._config.sort_thumbnails = self._sort_var.get()
            self._config.show_archive_icon = self._icon_var.get()
            self._config.collage_mode = self._collage_var.get()
            self._config.save()
            self._status_var.set(f"Applied {len(changes)} change(s)")
        else:
            # Revert UI to saved state
            for k, v in old_state.items():
                if k in self._format_vars:
                    self._format_vars[k].set(v)
            self._sort_var.set(self._config.sort_thumbnails)
            self._icon_var.set(self._config.show_archive_icon)
            self._collage_var.set(self._config.collage_mode)
            self._status_var.set("Changes reverted")

    def _save_config(self) -> None:
        for fmt_key, var in self._format_vars.items():
            self._config.enabled_formats[fmt_key] = var.get()
        if hasattr(self, "_sort_var"):
            self._config.sort_thumbnails = self._sort_var.get()
        if hasattr(self, "_icon_var"):
            self._config.show_archive_icon = self._icon_var.get()
        if hasattr(self, "_collage_var"):
            self._config.collage_mode = self._collage_var.get()
        self._config.save()
        self._status_var.set("Configuration saved")

    def _save_settings(self) -> None:
        try:
            self._config.thumbnail_size = int(self._size_var.get())
        except ValueError:
            pass
        self._config.dark_mode = self._theme_var.get()
        try:
            self._config.performance.max_workers = int(self._workers_var.get())
        except ValueError:
            pass
        self._config.log_level = self._log_var.get()
        self._config.save()
        self._status_var.set("Settings saved")

    def _refresh_stats(self) -> None:
        def _do():
            try:
                if self._engine is None:
                    from ..engine import ThumbnailEngine

                    self._engine = ThumbnailEngine(self._config)
                stats = self._engine.get_stats()
                cache = self._engine._cache  # noqa: access private for stats
                cache_stats = cache.stats if cache and hasattr(cache, "stats") else {}

                def _update():
                    self._perf_labels["total"].configure(text=str(stats.total))
                    self._perf_labels["succeeded"].configure(text=str(stats.succeeded))
                    self._perf_labels["failed"].configure(text=str(stats.failed))
                    avg_ms = stats.total_ms / max(1, stats.total)
                    self._perf_labels["avg_ms"].configure(text=f"{avg_ms:.1f} ms")
                    ips = stats.succeeded / max(0.001, stats.total_ms / 1000)
                    self._perf_labels["img_per_sec"].configure(text=f"{ips:.0f}")

                    self._cache_labels["mem_items"].configure(
                        text=str(
                            cache_stats.get("l1_items", cache_stats.get("items", "—"))
                        )
                    )
                    self._cache_labels["mem_mb"].configure(
                        text=f"{cache_stats.get('l1_memory_mb', cache_stats.get('memory_mb', 0)):.1f} MB"
                    )
                    self._cache_labels["disk_items"].configure(
                        text=str(cache_stats.get("l2_items", "—"))
                    )
                    self._cache_labels["disk_mb"].configure(
                        text=f"{cache_stats.get('l2_size_mb', cache_stats.get('size_mb', 0)):.1f} MB"
                    )
                    hit_rate = cache_stats.get("hit_rate", 0)
                    self._cache_labels["hit_rate"].configure(text=f"{hit_rate:.1f}%")
                    self._status_var.set("Stats refreshed")

                self._root.after(0, _update)
            except Exception as exc:
                self._root.after(0, lambda: self._status_var.set(f"Stats error: {exc}"))

        threading.Thread(target=_do, daemon=True).start()

    def _clear_cache(self) -> None:
        if messagebox.askyesno("Clear Cache", "Clear all cached thumbnails?"):
            if self._engine and self._engine._cache:
                self._engine._cache.clear()
            self._status_var.set("Cache cleared")

    def _register(self) -> None:
        def _do():
            from ..shell.com_server import is_admin, register

            if not is_admin():
                self._root.after(
                    0,
                    lambda: messagebox.showwarning(
                        "Admin Required", "Please run as administrator to register."
                    ),
                )
                return
            ok = register(self._config.get_enabled_extensions())
            msg = "Registration successful" if ok else "Registration failed"
            self._root.after(0, lambda: self._status_var.set(msg))

        threading.Thread(target=_do, daemon=True).start()

    def _unregister(self) -> None:
        def _do():
            from ..shell.com_server import is_admin, unregister

            if not is_admin():
                self._root.after(
                    0,
                    lambda: messagebox.showwarning(
                        "Admin Required", "Please run as administrator to unregister."
                    ),
                )
                return
            ok = unregister()
            msg = "Unregistration complete" if ok else "Unregistration failed"
            self._root.after(0, lambda: self._status_var.set(msg))

        threading.Thread(target=_do, daemon=True).start()

    def _elevate(self) -> None:
        """Re-launch self as admin."""
        try:
            ctypes.windll.shell32.ShellExecuteW(
                None, "runas", sys.executable, f'"{sys.argv[0]}"', None, 1
            )
        except Exception as exc:
            messagebox.showerror("Elevation Failed", str(exc))

    def _check_registration(self) -> None:
        def _do():
            from ..shell.com_server import get_registration_status

            status = get_registration_status()
            lines = []
            for ext, registered in sorted(status.items()):
                icon = "✓" if registered else "✗"
                lines.append(f"  {icon}  {ext}")
            text = "\n".join(lines) or "No extensions checked"
            self._root.after(0, lambda: self._update_reg_text(text))

        threading.Thread(target=_do, daemon=True).start()

    def _update_reg_text(self, text: str) -> None:
        self._reg_status_text.configure(state=tk.NORMAL)
        self._reg_status_text.delete("1.0", tk.END)
        self._reg_status_text.insert("1.0", text)
        self._reg_status_text.configure(state=tk.DISABLED)

    def _update_admin_status(self) -> None:
        try:
            is_admin = ctypes.windll.shell32.IsUserAnAdmin() != 0
        except Exception:
            is_admin = False
        if is_admin:
            self._admin_label.configure(
                text="✓ Running as Administrator",
                foreground="#4ec9b0" if self._dark_mode else "#008000",
            )
        else:
            self._admin_label.configure(
                text="✗ Not running as Administrator",
                foreground="#f44747" if self._dark_mode else "#cc0000",
            )

    def _export_config(self) -> None:
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json")],
            title="Export Configuration",
        )
        if path:
            self._config.export_json(path)
            self._status_var.set(f"Exported to {path}")

    def _export_diagnostics(self) -> None:
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json")],
            title="Export Diagnostics",
            initialfile="explorerlens-diagnostics.json",
        )
        if path:

            def _do():
                from ..shell.diagnostics import export_diagnostics

                out = export_diagnostics(Path(path))
                self._root.after(
                    0, lambda: self._status_var.set(f"Diagnostics exported to {out}")
                )

            threading.Thread(target=_do, daemon=True).start()

    def _detect_conflicts(self) -> None:
        def _do():
            from ..registry import detect_conflicts

            conflicts = detect_conflicts()
            if not conflicts:
                text = "No conflicts detected — no other handlers found."
            else:
                lines = [
                    f"  ! {ext} → {clsid}" for ext, clsid in sorted(conflicts.items())
                ]
                text = f"Found {len(conflicts)} conflicting handler(s):\n" + "\n".join(
                    lines
                )
            self._root.after(0, lambda: self._update_reg_text(text))

        threading.Thread(target=_do, daemon=True).start()

    def _flush_cache(self) -> None:
        if messagebox.askyesno(
            "Flush Explorer Cache",
            "Delete Windows thumbnail cache files?\n"
            "Explorer will regenerate thumbnails.",
        ):
            from ..registry import flush_thumbnail_cache

            ok = flush_thumbnail_cache()
            self._status_var.set("Explorer cache flushed" if ok else "Flush failed")

    def _import_config(self) -> None:
        path = filedialog.askopenfilename(
            filetypes=[("JSON files", "*.json")],
            title="Import Configuration",
        )
        if path:
            self._config = Config.import_json(path)
            self._refresh_ui()
            self._status_var.set(f"Imported from {path}")

    def _reset_defaults(self) -> None:
        if messagebox.askyesno("Reset", "Reset all settings to defaults?"):
            self._config.reset_defaults()
            self._config.save()
            self._refresh_ui()
            self._status_var.set("Settings reset to defaults")

    def _refresh_ui(self) -> None:
        """Refresh UI from current config."""
        for fmt_key, var in self._format_vars.items():
            var.set(self._config.enabled_formats.get(fmt_key, True))
        for group_key in self._group_all_vars:
            self._update_group_all_state(group_key)
        self._size_var.set(str(self._config.thumbnail_size))
        self._theme_var.set(self._config.dark_mode)
        self._workers_var.set(str(self._config.performance.max_workers))
        self._log_var.set(self._config.log_level)
        if hasattr(self, "_sort_var"):
            self._sort_var.set(self._config.sort_thumbnails)
        if hasattr(self, "_icon_var"):
            self._icon_var.set(self._config.show_archive_icon)
        if hasattr(self, "_collage_var"):
            self._collage_var.set(self._config.collage_mode)

    def _on_close(self) -> None:
        """Save config, stop tray icon, and close."""
        self._save_config()
        if self._tray_icon is not None:
            try:
                self._tray_icon.stop()
            except Exception:
                pass
        if self._engine is not None:
            self._engine.shutdown()
        self._root.destroy()

    def _setup_tray_icon(self) -> None:
        """Create a system tray icon with context menu."""
        try:
            import pystray
            from PIL import Image as PILImage

            # Create a simple icon (blue square with "EL")
            icon_img = PILImage.new("RGB", (64, 64), (14, 99, 156))
            from PIL import ImageDraw, ImageFont

            draw = ImageDraw.Draw(icon_img)
            try:
                font = ImageFont.truetype("segoeui.ttf", 28)
            except Exception:
                font = ImageFont.load_default()
            draw.text((8, 14), "EL", fill=(255, 255, 255), font=font)

            menu = pystray.Menu(
                pystray.MenuItem("Show", self._tray_show),
                pystray.MenuItem("Quit", self._tray_quit),
            )
            self._tray_icon = pystray.Icon(
                "ExplorerLens.py", icon_img, "ExplorerLens.py Manager", menu
            )
            threading.Thread(target=self._tray_icon.run, daemon=True).start()
        except ImportError:
            logger.debug("pystray not installed — tray icon disabled")
        except Exception as exc:
            logger.debug("Tray icon setup failed: %s", exc)

    def _tray_show(self, icon=None, item=None) -> None:
        """Show the main window from tray."""
        if self._root:
            self._root.after(0, self._root.deiconify)

    def _tray_quit(self, icon=None, item=None) -> None:
        """Quit from tray icon."""
        if self._tray_icon:
            self._tray_icon.stop()
        if self._root:
            self._root.after(0, self._on_close)
