import os
from pathlib import Path
from PIL import Image
import sys

def convert_jpeg_to_bmp(input_folder, output_folder=None):
    """
    Convert all JPEG images in a folder to BMP format.
    
    Args:
        input_folder (str): Path to folder containing JPEG images
        output_folder (str): Path to save BMP files. If None, saves to input_folder
    """
    
    # Validate input folder
    if not os.path.isdir(input_folder):
        print(f"Error: '{input_folder}' is not a valid directory.")
        return
    
    # Set output folder to input folder if not specified
    if output_folder is None:
        output_folder = input_folder
    else:
        # Create output folder if it doesn't exist
        os.makedirs(output_folder, exist_ok=True)
    
    # Find all JPEG files
    jpeg_files = list(Path(input_folder).glob('*.jpg')) + \
                 list(Path(input_folder).glob('*.jpeg')) + \
                 list(Path(input_folder).glob('*.JPG')) + \
                 list(Path(input_folder).glob('*.JPEG'))
    
    if not jpeg_files:
        print(f"No JPEG files found in '{input_folder}'")
        return
    
    print(f"Found {len(jpeg_files)} JPEG file(s). Starting conversion...\n")
    
    successful = 0
    failed = 0
    
    # Convert each JPEG to BMP
    for jpeg_file in jpeg_files:
        try:
            # Open the JPEG image
            img = Image.open(jpeg_file)

            # Resize to 480 x 800 (width x height)
            img = img.resize((800, 480), Image.LANCZOS)
            
            # Create output filename with .bmp extension
            output_filename = jpeg_file.stem + '.bmp'
            output_path = os.path.join(output_folder, output_filename)
            
            # Convert to RGB if necessary (some JPEGs might be RGBA)
            if img.mode in ('RGBA', 'LA', 'P'):
                rgb_img = Image.new('RGB', img.size, (255, 255, 255))
                rgb_img.paste(img, mask=img.split()[-1] if img.mode == 'RGBA' else None)
                rgb_img.save(output_path, 'BMP')
            else:
                img.save(output_path, 'BMP')
            
            print(f"✓ Converted: {jpeg_file.name} → {output_filename} (800x480)")
            successful += 1
            
        except Exception as e:
            print(f"✗ Failed to convert {jpeg_file.name}: {str(e)}")
            failed += 1
    
    # Summary
    print(f"\n--- Conversion Complete ---")
    print(f"Successful: {successful}")
    print(f"Failed: {failed}")
    print(f"Output folder: {output_folder}")

if __name__ == "__main__":
    # Get folder path from command line argument or prompt user
    if len(sys.argv) > 1:
        input_folder = sys.argv[1]
        output_folder = sys.argv[2] if len(sys.argv) > 2 else None
    else:
        input_folder = input("Enter the path to the JPEG folder: ").strip()
        output_choice = input("Enter output folder (press Enter to use same folder): ").strip()
        output_folder = output_choice if output_choice else None
    
    convert_jpeg_to_bmp(input_folder, output_folder)