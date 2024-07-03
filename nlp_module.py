import sys
import spacy
import json
import os
import re

nlp = spacy.load("en_core_web_sm")

# Load documentation from JSON file
json_file_path = os.path.join(os.path.dirname(__file__), "documentation.json")
with open(json_file_path, "r") as file:
    documentation = json.load(file)

def process_command(command):
    print(f"Processing command: {command}")  # Debugging print
    doc = nlp(command)
    tokens = [token.text for token in doc]
    print(f"Tokens: {tokens}")  # Debugging print
    
    # Convert command to lowercase for case-insensitive matching
    command_lower = command.lower()
    
    for lang_key, snippets in documentation.items():
        for key, doc_info in snippets.items():
            # Use regular expressions to match whole words only
            if re.search(r'\b' + re.escape(key.lower()) + r'\b', command_lower):
                print(f"Matched key: {key}, doc_info: {doc_info}")  # Debugging print
                return f"You entered: {command}\nDescription:\n{doc_info['description']}\nExample:\n{doc_info['example']}"
    
    return f"Command not recognized. Tokens: {tokens}"

if __name__ == "__main__":
    command = " ".join(sys.argv[1:])
    print(process_command(command))

