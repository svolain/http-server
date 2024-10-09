# to test form submissions for file uploads
import cgi
import os
import sys

UPLOAD_DIR = "../uploads"

print("Content-Type: text/html\n")

try:
    form = cgi.FieldStorage()
    name = form.getvalue("name")
    getGetfile_item = form["file"]

    if getGetfile_item.filename:
        filename = os.path.basename(getGetfile_item.filename)
        getGetfile_path = os.path.join(UPLOAD_DIR, filename)
        with open(getGetfile_path, "wb") as f:
            f.write(getGetfile_item.file.read())
            #file uploaded successfully
    else:
        print("Error: file upload failed")
except Exception as e:
    print(f"Error occurred: {e}")
    sys.exit(1)


