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

def identify_language(command):
    # Heuristics to identify language based on common keywords
    if re.search(r'\b(print|def|for|import|if|in)\b', command):
        return "Python"
    if re.search(r'\b(#include|std::cout|int main|printf)\b', command):
        return "C++"
    if re.search(r'\b(console.log|function|let|const|var)\b', command):
        return "JavaScript"
    if re.search(r'\b(let|match|type|describe_list)\b', command):
        return "OCaml"
    # Add more heuristics for other languages as needed
    return "Unknown"

def match_command(command, language):
    for key, doc_info in documentation.get(language, {}).items():
        if re.search(r'\b' + re.escape(key.lower()) + r'\b', command.lower()):
            return doc_info
    return None

def process_command(command):
    print(f"Processing command: {command}")  # Debugging print
    doc = nlp(command)
    tokens = [token.text for token in doc]
    print(f"Tokens: {tokens}")  # Debugging print
    
    # Identify the language of the code
    language = identify_language(command)
    
    if language != "Unknown":
        # Search for documentation within the identified language
        doc_info = match_command(command, language)
        if doc_info:
            print(f"Matched key: {command}, doc_info: {doc_info}")  # Debugging print
            return (language, f"You entered: {command}\n\nLanguage: {language}\n\nDescription:\n{doc_info['description']}\n\nExample:\n{doc_info['example']}")
    
    # Handle combined variable assignment and loops
    if "=" in tokens and ("for" in tokens or "while" in tokens):
        # Handle combined statements
        assignment_part = command.split("for")[0].strip() if "for" in tokens else command.split("while")[0].strip()
        loop_part = "for " + command.split("for")[1].strip() if "for" in tokens else "while " + command.split("while")[1].strip()
        assignment_info = process_command(assignment_part)
        loop_info = process_command(loop_part)
        return (language, f"You entered: {command}\n\nLanguage: {language}\n\nDescription:\nAssigns a value to a variable and contains a loop.\n\nExample:\n{assignment_info[1]}\n{loop_info[1]}")

    # Handle simple variable assignment
    if "=" in tokens:
        variable_name = tokens[0]
        value = command.split('=')[1].strip()
        if language == "Unknown":
            if re.match(r'["\'].*["\']|\[.*\]', value):
                language = "Python"
            else:
                language = "Unknown"
        doc_info = {"description": "Assigns a value to a variable.", "example": f"{variable_name} = {value}"}
        return (language, f"You entered: {command}\n\nLanguage: {language}\n\nDescription:\n{doc_info['description']}\n\nExample:\n{doc_info['example']}")
    
    return (language, f"Command not recognized. Tokens: {tokens}")

if __name__ == "__main__":
    command = " ".join(sys.argv[1:])
    language, result = process_command(command)
    print(result)
