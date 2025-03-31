import sys

def write_to_file(text_to_write):
    file_path = 'zzfixed_file.txt'
    try:
        with open(file_path, 'w') as file:
            file.write(str(text_to_write))
        print(f'Successfully wrote to {file_path}')
    except Exception as e:
        print(f'Error writing to file: {e}')

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <text_to_write>")
        sys.exit(1)

    text_to_write = sys.argv[1]

    write_to_file(text_to_write)

