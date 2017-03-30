import hashlib
from random import randint

with open('fname.txt','r') as fname:
    fnames = fname.readlines()
with open('lname.txt','r') as lname:
    lnames = lname.readlines()

for i in range(0, randint(200,350)):
    fname = fnames[randint(0,len(fnames))][:-1]
    lname = lnames[randint(0,len(lnames))][:-1]
    m = hashlib.md5()
    m.update(os.urandom(5000))
    password = m.hexdigest()

    #stdout
    with open("dump.txt",'a+') as dumpfile:
        dumpfile.write(" ".join(["|", fname, "|", lname, "|", "%s.%s@wtiiaas.csg" % (lname.lower(), fname.lower()), "|", password, "|", '\n']))

    #sql
    print("INSERT INTO `employees` (`employee_id`, `employee_fname`, `employee_lname`, `employee_email`, `employee_password`, `employee_phonenumber`, `employee_role`) VALUES (NULL, '%s', '%s', '%s.%s@wtiiaas.csg', '%s', '%d-%d-%d', '5');" % (lname, fname, lname.lower(), fname.lower(), password, randint(100,999), randint(100,999), randint(1000,9999)))
