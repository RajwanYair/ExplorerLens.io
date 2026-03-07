from setuptools import setup, find_packages

setup(
    name="explorerlens",
    version="1.0.0",
    description="Python Windows Shell Extension thumbnail provider — 200+ formats",
    long_description=open("README.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    author="ExplorerLens Project",
    license="MIT",
    python_requires=">=3.11",
    packages=find_packages(),
    entry_points={
        "console_scripts": [
            "explorerlens=explorerlens.__main__:main",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Environment :: Win32 (MS Windows)",
        "Operating System :: Microsoft :: Windows :: Windows 10",
        "Programming Language :: Python :: 3.11",
        "Topic :: Desktop Environment :: File Managers",
    ],
)
