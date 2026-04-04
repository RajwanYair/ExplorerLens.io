'use strict';

// Downloads lens.exe from GitHub Releases during npm install.
// Skips silently on non-Windows platforms.

const https = require('https');
const fs = require('fs');
const path = require('path');

if (process.platform !== 'win32') {
  process.stdout.write('ExplorerLens lens.exe is Windows-only — skipping binary download.\n');
  process.exit(0);
}

const pkg = require('../package.json');
const version = pkg.version;
const vendorDir = path.join(__dirname, '..', 'vendor');
const destPath = path.join(vendorDir, 'lens.exe');

if (fs.existsSync(destPath)) {
  process.exit(0);
}

const url =
  `https://github.com/RajwanYair/ExplorerLens.io/releases/download/v${version}/lens.exe`;

fs.mkdirSync(vendorDir, { recursive: true });
process.stdout.write(`Downloading ExplorerLens lens.exe v${version}...\n`);

function download(url, dest, hops) {
  if (hops > 5) {
    process.stderr.write('Too many redirects — aborting lens.exe download.\n');
    process.exit(0); // non-fatal: user can download manually
  }
  https.get(url, (res) => {
    if (res.statusCode === 301 || res.statusCode === 302) {
      return download(res.headers.location, dest, hops + 1);
    }
    if (res.statusCode !== 200) {
      process.stderr.write(
        `Download failed (HTTP ${res.statusCode}). ` +
        'Get lens.exe manually from https://github.com/RajwanYair/ExplorerLens.io/releases\n'
      );
      process.exit(0); // non-fatal
    }
    const file = fs.createWriteStream(dest);
    res.pipe(file);
    file.on('finish', () => {
      file.close();
      process.stdout.write('lens.exe ready.\n');
    });
  }).on('error', (err) => {
    process.stderr.write(
      `Network error: ${err.message}\n` +
      'Get lens.exe manually from https://github.com/RajwanYair/ExplorerLens.io/releases\n'
    );
    process.exit(0); // non-fatal
  });
}

download(url, destPath, 0);
