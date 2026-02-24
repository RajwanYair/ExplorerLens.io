#!/usr/bin/env python3
"""
ExplorerLens Test Data Generator
Creates sample archive files for integration testing
"""

import os
import zipfile
import tarfile
from pathlib import Path
from io import BytesIO

# Simple 1x1 pixel images in various formats (Base64 encoded)
# These are minimal valid image files for testing

# 1x1 Red pixel PNG
PNG_DATA = bytes.fromhex(
    '89504e470d0a1a0a0000000d494844520000000100000001'
    '08020000009077e0f50000000c49444154089963600100000500010d0a2db40000000049454e'
    '44ae426082'
)

# 1x1 Red pixel BMP
BMP_DATA = bytes.fromhex(
    '424d3a0000000000000036000000280000000100000001000000'
    '0100180000000000040000000000000000000000000000000000'
    '00000000ff0000'
)

# 1x1 Red pixel GIF
GIF_DATA = bytes.fromhex(
    '474946383961010001000000ff0000002c00000000010001000002'
    '0144003b'
)

# 1x1 Red pixel JPEG
JPEG_DATA = bytes.fromhex(
    'ffd8ffe000104a46494600010100000100010000ffdb004300080606070605080707'
    '0707090909080a0c140d0c0b0b0c1912130f141d1a1f1e1d1a1c1c20242e2720222c'
    '231c1c2837292c30313434341f27393d38323c2e333432ffdb0043010909090c0b0c'
    '180d0d1832211c213232323232323232323232323232323232323232323232323232'
    '32323232323232323232323232323232323232323232ffc00011080001000103011100'
    '0211010311010ffc4001500010501010101010100000000000000000102030405060708'
    '090a0bffc400b5100002010303020403050504040000017d01020300041105122131410613'
    '5161071322328108144291a1b1c109233352f0156272d10a162434e125f11718191a262728'
    '292a35363738393a434445464748494a535455565758595a636465666768696a7374757677'
    '78797a838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9'
    'bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1f2f3f4f5f6f7f8'
    'f9faffc4001f0100030101010101010101010000000000000102030405060708090a0bffc4'
    '00b51100020102040403040705040400010277000102031104052131061241510761711322'
    '328108144291a1b1c109233352f0156272d10a162434e125f11718191a262728292a353637'
    '38393a434445464748494a535455565758595a636465666768696a737475767778797a8283'
    '8485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4'
    'c5c6c7c8c9cad2d3d4d5d6d7d8d9dae2e3e4e5e6e7e8e9eaf2f3f4f5f6f7f8f9faffda000c'
    '03010002110311003f00ffda0008010100003f00d2ffda000c03010002110311003f00ffda'
    '0008010301013f00f7ffda0008010201013f00f7ffda0008010100063f02ff0049ff006aff'
    '00db0049ffd9'
)

class TestDataGenerator:
    def __init__(self, output_dir="test_data"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        
    def create_test_images(self):
        """Create test image files in multiple formats"""
        print("Creating test images...")
        
        images = {
            'test_01.png': PNG_DATA,
            'test_02.bmp': BMP_DATA,
            'test_03.gif': GIF_DATA,
            'test_04.jpg': JPEG_DATA,
            'cover.png': PNG_DATA,
            'page_01.jpg': JPEG_DATA,
            'page_02.jpg': JPEG_DATA,
            'page_03.png': PNG_DATA,
        }
        
        for filename, data in images.items():
            path = self.output_dir / filename
            path.write_bytes(data)
            print(f"  Created: {filename} ({len(data)} bytes)")
            
        return list(images.keys())
    
    def create_cbz_file(self, images):
        """Create a test CBZ (Comic Book ZIP) file"""
        print("\nCreating CBZ test file...")
        
        cbz_path = self.output_dir / "test_comic.cbz"
        with zipfile.ZipFile(cbz_path, 'w', zipfile.ZIP_DEFLATED) as zf:
            for img in images:
                img_path = self.output_dir / img
                zf.write(img_path, img)
        
        print(f"  Created: test_comic.cbz ({cbz_path.stat().st_size} bytes)")
        return cbz_path
    
    def create_zip_file(self, images):
        """Create a test ZIP file"""
        print("Creating ZIP test file...")
        
        zip_path = self.output_dir / "test_archive.zip"
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:
            for img in images[:4]:  # First 4 images
                img_path = self.output_dir / img
                zf.write(img_path, img)
        
        print(f"  Created: test_archive.zip ({zip_path.stat().st_size} bytes)")
        return zip_path
    
    def create_epub_file(self, images):
        """Create a minimal EPUB file for testing"""
        print("Creating EPUB test file...")
        
        epub_path = self.output_dir / "test_ebook.epub"
        
        # EPUB is a ZIP with specific structure
        with zipfile.ZipFile(epub_path, 'w', zipfile.ZIP_DEFLATED) as zf:
            # mimetype (uncompressed, first file)
            zf.writestr('mimetype', 'application/epub+zip', compress_type=zipfile.ZIP_STORED)
            
            # META-INF/container.xml
            container_xml = '''<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/>
  </rootfiles>
</container>'''
            zf.writestr('META-INF/container.xml', container_xml)
            
            # OEBPS/content.opf
            content_opf = '''<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="2.0" unique-identifier="BookID">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:title>Test Book</dc:title>
    <dc:creator>Test Author</dc:creator>
    <dc:identifier id="BookID">test-001</dc:identifier>
    <dc:language>en</dc:language>
    <meta name="cover" content="cover-image"/>
  </metadata>
  <manifest>
    <item id="cover-image" href="images/cover.png" media-type="image/png"/>
    <item id="page1" href="images/page_01.jpg" media-type="image/jpeg"/>
  </manifest>
  <spine>
    <itemref idref="cover-image"/>
  </spine>
</package>'''
            zf.writestr('OEBPS/content.opf', content_opf)
            
            # Add cover image
            cover_path = self.output_dir / 'cover.png'
            zf.write(cover_path, 'OEBPS/images/cover.png')
            
            # Add page image
            page_path = self.output_dir / 'page_01.jpg'
            zf.write(page_path, 'OEBPS/images/page_01.jpg')
        
        print(f"  Created: test_ebook.epub ({epub_path.stat().st_size} bytes)")
        return epub_path
    
    def create_tar_file(self, images):
        """Create a test TAR file"""
        print("Creating TAR test file...")
        
        tar_path = self.output_dir / "test_archive.tar"
        with tarfile.open(tar_path, 'w') as tf:
            for img in images[:3]:  # First 3 images
                img_path = self.output_dir / img
                tf.add(img_path, arcname=img)
        
        print(f"  Created: test_archive.tar ({tar_path.stat().st_size} bytes)")
        return tar_path
    
    def create_cbt_file(self, images):
        """Create a test CBT (Comic Book TAR) file"""
        print("Creating CBT test file...")
        
        cbt_path = self.output_dir / "test_comic.cbt"
        with tarfile.open(cbt_path, 'w') as tf:
            for img in images:
                img_path = self.output_dir / img
                tf.add(img_path, arcname=img)
        
        print(f"  Created: test_comic.cbt ({cbt_path.stat().st_size} bytes)")
        return cbt_path
    
    def create_phz_file(self, images):
        """Create a test PHZ (Photo ZIP) file"""
        print("Creating PHZ test file...")
        
        phz_path = self.output_dir / "test_photos.phz"
        with zipfile.ZipFile(phz_path, 'w', zipfile.ZIP_DEFLATED) as zf:
            for img in images[:5]:
                img_path = self.output_dir / img
                zf.write(img_path, img)
        
        print(f"  Created: test_photos.phz ({phz_path.stat().st_size} bytes)")
        return phz_path
    
    def create_readme(self):
        """Create README for test data"""
        readme_content = """# ExplorerLens Test Data

This directory contains test files for ExplorerLens unit and integration testing.

## Test Files

### Images
- `*.png` - PNG test images (minimal 1x1 pixel)
- `*.jpg` - JPEG test images (minimal 1x1 pixel)
- `*.bmp` - BMP test images (minimal 1x1 pixel)
- `*.gif` - GIF test images (minimal 1x1 pixel)

### Archives
- `test_comic.cbz` - Comic Book ZIP format
- `test_archive.zip` - Standard ZIP archive
- `test_ebook.epub` - EPUB ebook format
- `test_archive.tar` - TAR archive
- `test_comic.cbt` - Comic Book TAR format
- `test_photos.phz` - Photo ZIP format

## Usage

These files are used by:
1. Unit tests (`UnitTests.cpp`) - Format detection
2. Integration tests - Thumbnail extraction
3. Manual testing - Windows Explorer integration

## Regeneration

To regenerate all test files:
```
python generate_test_data.py
```

## Notes

- All images are minimal valid files (1x1 pixel)
- Archives contain sample images
- EPUB has proper structure per specification
- Files are suitable for automated testing
"""
        
        readme_path = self.output_dir / "README.md"
        readme_path.write_text(readme_content, encoding='utf-8')
        print(f"\n  Created: README.md")
    
    def cleanup_temp_images(self, images):
        """Remove temporary individual image files"""
        print("\nCleaning up temporary files...")
        for img in images:
            img_path = self.output_dir / img
            if img_path.exists():
                img_path.unlink()
                print(f"  Removed: {img}")
    
    def generate_all(self, keep_images=True):
        """Generate all test data"""
        print("=" * 50)
        print("ExplorerLens Test Data Generator")
        print("=" * 50)
        print()
        
        # Create images
        images = self.create_test_images()
        
        # Create archives
        self.create_cbz_file(images)
        self.create_zip_file(images)
        self.create_epub_file(images)
        self.create_tar_file(images)
        self.create_cbt_file(images)
        self.create_phz_file(images)
        
        # Create README
        self.create_readme()
        
        # Optionally cleanup individual images
        if not keep_images:
            self.cleanup_temp_images(images)
        
        print()
        print("=" * 50)
        print("Test Data Generation Complete!")
        print(f"Output directory: {self.output_dir.absolute()}")
        print("=" * 50)

if __name__ == "__main__":
    generator = TestDataGenerator()
    generator.generate_all(keep_images=True)

