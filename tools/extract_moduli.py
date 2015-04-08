import sys
import os
import subprocess
import OpenSSL

input = open('certs', 'r')
output = open('certs_moduli', 'w')


for l in input.readlines():
	tmpf = open('tmp.pem', 'w')
	tmpf.write('-----BEGIN CERTIFICATE-----\n')
	tmpf.write(l)
	tmpf.write('-----END CERTIFICATE-----')
	tmpf.close()
	
	p = subprocess.Popen('fold -w 64 tmp.pem',stdout=subprocess.PIPE,stderr=subprocess.PIPE, shell=True)
	output, errors = p.communicate()

	print "folded cert =", output

	os.remove('tmp.pem')

	tmpf = open('tmp.pem', 'w')
	tmpf.write(output)
	tmpf.close()

	p = subprocess.Popen('openssl x509 -noout -in tmp.pem -modulus', stdout=subprocess.PIPE,stderr=subprocess.PIPE, shell=True)
	output, errors = p.communicate()

	print 'errors =', errors
	print output
	

input.close()
output.close()
