# to test form submissions for file uploads
import cgi
import os
import sys

UPLOAD_DIR = "../uploads"

print("Content-Type: text/html\n")

try:
    form = cgi.FieldStorage()
    name = form.getvalue("name")
    getfile_item = form["file"]

    if getfile_item.filename:
        filename = os.path.basename(getfile_item.filename)
        getfile_path = os.path.join(UPLOAD_DIR, filename)
        with open(getfile_path, "wb") as f:
            f.write(getfile_item.file.read())
            #file uploaded successfully
    else:
        print("Error: file upload failed")
except Exception as e:
    print(f"Error occurred: {e}")
    sys.exit(1)


