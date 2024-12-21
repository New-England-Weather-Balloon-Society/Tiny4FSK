# Simple script to take KiCAD Centroid files and convert to JLCPCB format


import csv

def rename_csv_header(input_file, output_file, new_header):
    # Read the CSV file
    with open(input_file, 'r', newline='') as csvfile:
        reader = csv.reader(csvfile)
        rows = list(reader)
        
        # Rename the first header row
        if rows:
            rows[0] = new_header
    
    # Write the updated rows to the new CSV file
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerows(rows)

# Example usage
input_file = 'Tiny4FSK-top-pos.csv'
output_file = 'centroid.csv'
new_header = ['Designator', 'Val', 'Package', 'Mid X', 'Mid Y', 'Rotation', 'Layer']  # Replace with the new header names

rename_csv_header(input_file, output_file, new_header)
