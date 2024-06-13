import sys

# Access command-line arguments
script_name = sys.argv[0]
arg1 = sys.argv[1] if len(sys.argv) > 1 else None
arg2 = sys.argv[2] if len(sys.argv) > 2 else None
arg3 = sys.argv[3] if len(sys.argv) > 3 else None

# Print command-line arguments
print("Script Name:", script_name)
print("Argument 1:", arg1)
print("Argument 2:", arg2)
print("Argument 3:", arg3)