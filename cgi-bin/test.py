#!/usr/bin/env python3

import os

def generate_html_response():
    print("Content-Type: text/html\r\n\r")
    print("<html>")
    print("<head><title>Environment Variables</title></head>")
    print("<body>")
    print("<h1>Environment Variables</h1>")
    print("<table border='1'>")
    print("<tr><th>Variable</th><th>Value</th></tr>")
    
    # Loop through each environment variable
    for key, value in os.environ.items():
        print(f"<tr><td>{key}</td><td>{value}</td></tr>")
    
    print("</table>")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    generate_html_response()
