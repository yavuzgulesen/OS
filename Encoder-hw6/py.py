

import glob, os
#os.chdir("/mydir")




def encrypt(text,s):
	result = ""
	# transverse the plain text
	for i in range(len(text)):
		char = text[i]
		# Encrypt uppercase characters in plain text

		if (char.isupper()):
			result += chr((ord(char) + s-65) % 26 + 65)
		# Encrypt lowercase characters in plain text
		else:
			result += chr((ord(char) + s - 97) % 26 + 97)
	return result


f= open("cipher.txt","w+")
for file in glob.glob("*.txt"):
	g = open(file,"r")
	g1 = g.readlines()
	for x in g1:
		f.write(encrypt(x,len(file)))



f.write("\nPath at terminal when executing this file")
f.write(os.getcwd() + "\n")
f.write(__file__ + "\n")
f.write("This file full path (following symlinks)")
full_path = os.path.realpath(__file__)
f.write(full_path + "\n")
f.write("This file directory and name")
path, filename = os.path.split(full_path)
f.write(path + ' --> ' + filename + "\n")
f.write("This file directory only")
f.write(os.path.dirname(full_path))
f.write("\nGenerated:")
f.write(file)


f.close()
g.close()