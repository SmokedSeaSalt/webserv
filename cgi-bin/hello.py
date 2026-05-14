#!/usr/bin/python
import sys
sys.stdout.write("Content-type:text/html\r\n")
sys.stdout.write("\r\n")

sys.stdout.write('<html>')
sys.stdout.write('<head>')
sys.stdout.write('<title>Hello from CGI in Python</title>')
sys.stdout.write('</head>')
sys.stdout.write('<body>')
sys.stdout.write('<h1>Hello World!</h2>')
sys.stdout.write('</body>')
sys.stdout.write('</html>')

#open("myFile.txt", "x")