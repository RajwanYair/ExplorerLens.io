from setuptools import find_packages, setup

setup(
    name="explorerlens",
    version="15.0.0",
    description=("Python Windows Shell Extension thumbnail provider" " — 200+ formats"),
    long_description=open("README.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    author="ExplorerLens Project",
    license="MIT",
    python_requires=">=3.11",
    packages=find_packages(exclude=["tests"]),
    entry_points={
        "console_scripts": [
            "explorerlens=explorerlens.__main__:main",
        ],
    },
    install_requires=[
        "Pillow>=10.4.0",
        "pillow-heif>=0.18.0",
        "mutagen>=1.47.0",
        "platformdirs>=4.2.0",
    ],
    extras_require={
        "full": [
            "rawpy>=0.21.0",
            "pymupdf>=1.24.0",
            "python-pptx>=1.0.0",
            "python-docx>=1.1.0",
            "cairosvg>=2.7.0",
            "fonttools>=4.50.0",
            "py7zr>=0.22.0",
            "rarfile>=4.2",
            "trimesh>=4.4.0",
            "ffmpeg-python>=0.2.0",
            "pywin32>=306",
            "comtypes>=1.4.0",
            "pystray>=0.19.0",
            "darkdetect>=0.8.0",
            "psutil>=5.9.0",
            "watchdog>=4.0.0",
        ],
        "dev": [
            "pytest>=8.0.0",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Environment :: Win32 (MS Windows)",
        "Operating System :: Microsoft :: Windows :: Windows 10",
        "Operating System :: Microsoft :: Windows :: Windows 11",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Topic :: Desktop Environment :: File Managers",
        "Topic :: Multimedia :: Graphics",
    ],
)
