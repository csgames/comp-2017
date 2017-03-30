# Systèmes d’exploitation

Le module `startup_simulator.ko` est un simulateur de startup de dernier cri s’exécutant dans le noyau Linux (une simulation doit être rapide, et il est bien connu que tout est plus rapide dans le noyau). On peut démarrer la simulation d’une startup en y fournissant le modèle d’affaires au format PDF, et après des calculs brevetés sur le contenu de ce dernier, la simulation prédit avec une précision révolutionnaire la valeur nette de l’entreprise pour les 16 prochaines années.

Le module peut être contrôlé au travers du fichier spécial `/dev/startup_simulator`. Votre objectif est de créer un programme qui contrôle le simulateur et qui expose ses fonctionnalités au travers d’un socket TCP, afin de vous-même devenir riches en vendant l’accès à des entrepreneurs ~~naïfs~~visonnaires.

Vous pouvez utiliser le langage de votre choix pour accomplir cette tâche. Notez qu’un squelette de programme C, qui établit une connexion réseau minimale conforme aux spécifications et fournit les structures nécessaires à la communication avec le pilote, vous est fourni.

## Interface client: requêtes

> Pour vous aider, l'archive de compétition contient le script `startup_simulator.py`, qui implémente un client que vous pouvez utiliser pour tester votre serveur. Vous pouvez également vous référer à son implémentation.

Le serveur doit écouter sur le port 4321.

Le protocole de communication avec le client TCP est basé sur l’utilisation de commandes synchrones: le client envoie une requête et le serveur y répond. Les connexions sont terminées sans cérémonie particulière; il suffit qu’une partie ou l’autre ferme son socket. Chaque connexion ne gère qu’une entreprise: les connexions ne sont pas réutilisables.

**Toutes les valeurs entières sont envoyées en ordre réseau** (voyez `man htonl` si vous n'êtes pas familiers avec cette notion). Ce fait sera réitéré à chaque occasion dans ce document.

L’en-tête des commandes et réponses est constitué de deux champs dont la taille totale est 4 octets:

* 2 octets: taille complète de la commande (en-tête compris), octets en ordre réseau;
* 2 octets: type de la commande, octets en ordre réseau.

La taille du corps de la commande est donc donnée par (taille complète - 4).

Les données du corps doivent être interprétées selon le type de la commande. Le client peut envoyer 2 commandes:

* `BEGIN_STARTUP` (0)
* `BUSINESS_PLAN_DATA` (1)

### BEGIN_STARTUP

Le corps de la commande `BEGIN_STARTUP` commence par le capital, en dollars, dont dispose l’entreprise à son démarrage. Ce nombre est encodé sur 4 octets en ordre réseau. Le reste du corps est le nom, encodé en UTF-8 et sans caractère nul final, de l’entreprise en démarrage. Le nom doit être long de 99 octets au maximum.

(20 octets, type 0; entreprise “Smartsuppose”, capital initial de 0x8344 $)

	00000000  00 14:00 00:00 00 83 44: 53 6d 61 72 74 73 75 70  |.......DSmartsup|
	00000010  70 6f 73 65                                       |pose|
	
Les connexions n’étant pas réutilisables, il serait une erreur d’envoyer un second message `BEGIN_STARTUP`. Il est également une erreur d'envoyer n'importe quelle autre commande tant qu'une commande `BEGIN_STARTUP` n'a pas été reçue.

### BUSINESS_PLAN_DATA

Le corps de la commande `BUSINESS_PLAN_DATA` est une portion du plan d’affaires, au format PDF, de l’entreprise. Le client doit envoyer cette commande autant de fois que nécessaire pour compléter le transfert du plan d’affaires. Il signale la fin du transfert en envoyant un paquet de ce type ne contenant pas de données.

La taille de la portion est déterminée par la taille de la commande.

	00000000  1c 4f:00 01:25 50 44 46  2d 31 2e 35 0d 25 e2 e3  |.O..%PDF-1.5.%..|
	00000010  cf d3 0d 0a 31 39 38 20  30 20 6f 62 6a 0d 3c 3c  |....198 0 obj.<<|
	00000020  2f 4c 69 6e 65 61 72 69  7a 65 64 20 31 2f 4c 20  |/Linearized 1/L |
	00000030  ...

## Interface client: réponses

Le serveur doit répondre au client suivant le même format d’en-tête. Les identifiants de commandes du serveur sont:

* `ACKNOWLEDGED` (2)
* `ANGEL_INVESTOR` (3)
* `PREDICTION_RESULT` (4)
* `ERROR` (5)

### ACKNOWLEDGED

La réponse `ACKNOWLEDGED` indique au client qu’il peut continuer ses envois. C’est la réponse habituelle aux commandes `BEGIN_STARTUP` et `BUSINESS_PLAN_DATA`. La réponse n'a pas de corps.

    00000000  00 04 00 02                                       |....|

### ANGEL_INVESTOR

La réponse `ANGEL_INVESTOR` indique au client que la simulation a déterminé, quelque part depuis la fin du dernier envoi, qu’un investisseur-ange s’intéressera tôt au projet et le transformera radicalement. Ainsi, la connexion doit être arrêtée. Cette réponse n’a pas de corps et termine la connexion.

    00000000  00 04 00 03                                       |....|

### PREDICTION_RESULT

La réponse `PREDICTION_RESULT` est utilisée lorsque le client a fini d’envoyer son plan d’affaires et que la simulation a terminé son exécution. La réponse doit comprendre la valeur des résultats prédits, octets en ordre réseau. Référez-vous à l'*Interface de contrôle* pour plus d'informations sur le résultat de la simulation. Cette réponse termine la connexion.

(l’entreprise a échoué et sa valeur nette tombe rapidement à 0:)

	00000000  00 84:00 04:00 00 01 00  00 00 02 00:00 00 03 00  |.C..............|
	00000010  00 04 00 00:00 05 01 00  00 06 01 00:00 08 01 22  |..............."|
	00000020  00 08 f2 40:00 00 00 00  00 00 00 00:00 00 00 00  |...@............|
	00000030  00 00 00 00:00 00 00 00  00 00 00 00:00 00 00 00  |................|
	00000040  ...                                               |...|
	
> **ATTENTION!** Si vous représentez ce paquet comme structure C, considérez que l'alignement natif des entiers de 64 bits pourrait laisser un trou de 4 octets entre l'en-tête du paquet et le premier résultat.

### ERROR

La réponse `ERROR` indique qu’une erreur est survenue. C'est le mécanisme standard par lequel le serveur rapporte au client les erreurs qu'il détecte, soit dans son état interne ou parce que le client demande une opération impossible. Le corps de la réponse doit inclure une chaîne de caractères expliquant de votre mieux l’erreur qui s’est produite. Cette chaîne ne devrait pas être terminée par un caractère nul. Les erreurs provoquées par le client (par exemple, s'il tente d'envoyer une partie de son plan d'affaires avant de démarrer la simulation) ne doivent pas terminer la connexion.

	00000000  00 1f:00 05:53 70 6c 69  6e 65 20 72 65 74 69 63  |....Spline retic|
	00000010  75 6c 61 74 69 6f 6e 20  66 61 69 6c 65 64 21     |ulation failed!|

## Interface de contrôle

Votre première action devrait être d’ouvrir `/dev/startup_simulator` en mode lecture et écriture. Vous pouvez ensuite utiliser l’appel système `ioctl` pour contrôler l’état du pilote.

Notez que bien que le fichier spécial puisse être ouvert plusieurs fois, par un ou plusieurs processus, il n’y existe qu’une seule instance du simulateur et tous les programmes communiquent avec. Par conséquent, si vous ouvrez le fichier deux fois et utilisez l’ioctl `CSGAMES_SET_STARTUP` sur le premier descripteur de fichier, les changements seront aussi visibles via le second descripteur de fichier, même dans différents processus.

En plus des opérations de lecture et d'écriture normales, le fichier spécial répond aux appels ioctl suivants:

* `CSGAMES_NEW_STARTUP` (0x100)
* `CSGAMES_SET_STARTUP` (0x101)
* `CSGAMES_GET_PROCESSING_END` (0x102)
* `CSGAMES_GET_ANGELS_PICK` (0x103)

### CSGAMES_NEW_STARTUP

L’ioctl `CSGAMES_NEW_STARTUP` accepte comme paramètre l’adresse d’une structure startup:

	struct startup {
		char name[100];
		unsigned capital_in_dollars;
	};

L’appel démarre la simulation d’une nouvelle entreprise, avec le nom et le capital déterminé, et sélectionne cette entreprise comme entreprise active (voir `CSGAMES_SET_STARTUP`). La simulation est terminée, et les ressources libérées, lorsque les résultats sont lus (voir opérations de lecture et d’écriture).
Le module peut gérer jusqu’à 8 entreprises simultanément.

Erreurs possibles:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide);
* `EINVAL` (nom incorrect);
* `ENFILE` (8 startups déjà actives).

### CSGAMES_SET_STARTUP

L’ioctl `CSGAMES_SET_STARTUP` détermine l’entreprise sur laquelle les opérations de lecture et d’écriture prennent effet. Il accepte comme paramètre une chaîne de caractères contenant le nom de l’entreprise, terminée par un caractère nul.

Erreurs possibles:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide)
* `ENOENT` (aucune entreprise trouvée avec le nom donné).

### CSGAMES_GET_PROCESSING_END

L’ioctl `CSGAMES_GET_PROCESSING_END` accepte comme paramètre un pointeur vers une structure de type `timeval`. La simulation prend du temps, et le pilote poursuit le traitement des données reçues en arrière-plan. L’appel copiera vers cette structure le moment où le simulateur sera prêt à accepter une nouvelle portion du plan d’affaires (voir opérations de lecture et d’écriture). Notez que vous devez respecter cette limite: si vous tentez une nouvelle opération de lecture ou écriture avant la fin de ce délai, vous pourriez augmenter le temps nécessaire à l’exécution de la simulation.

Erreurs possibles:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide)
* `EINVAL` (aucune startup sélectionnée).

### CSGAMES_GET_ANGELS_PICK

L’ioctl `CSGAMES_GET_ANGELS_PICK` accepte comme paramètre une zone mémoire d’au moins 100 octets de long et y place le nom de l’entreprise choisie par un investisseur-ange (voir `SIGNGEL` plus bas). Cette commande libère les ressources associées à la simulation de l’entreprise cible.

Au terme de l’appel, aucune startup n’est sélectionnée pour les prochaines opérations de lecture ou écriture.

Erreurs possibles:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide)
* `EAGAIN` (aucune startup choisie).

### Opérations de lecture et d’écriture

Les opérations de lecture et d’écriture s’appliquent à l’entreprise sélectionnée par l’ioctl `CSGAMES_SET_STARTUP`.

L’opération d’écriture doit fournir les données du plan d’affaires que le client transmet. Notez que le fichier doit être au format PDF pour être accepté par le simulateur.

Le simulateur n’offre *pas* de forte garantie sur la quantité de données qui seront acceptées à chaque appel d’écriture. En d'autres mots, un appel à `write` qui passe N octets risque de retourner une valeur R plus petite que N, signifiant que seulement R des N octets ont été acceptés sur le coup. Votre programme doit tenir compte de ce comportement et utiliser un autre appel à `write` pour repasser les données qui n'ont pas été lues par l'appel précédent.

Pour des raisons techniques complexes, le pilote ne peut pas accepter moins de 4 octets ou plus de 32768 octets à la fois. Ceci dit, dépendant du style du plan d’affaires, il est fort probable que le maximum ne soit que rarement atteint.

Sachez également qu’une opération d’écriture impose un délai de traitement à la simulation. En général, il vous sera impossible de lire ou d’écrire vers cette simulation pour quelques secondes après y avoir écrit des données. Pendant ce délai, il n’est pas recommandé de tenter de lire ou d’écrire plus de données vers cette simulation d’entreprise, puisqu’elle pourrait en être ralentie. (Rien ne vous empêche, cependant, d’utiliser l’ioctl `CSGAMES_SET_STARTUP` pour modifier une autre simulation entre-temps.)

La fin du délai de simulation pour l’entreprise courante peut être obtenue, sous forme d’une structure timeval, à l’aide de l’ioctl `CSGAMES_GET_PROCESSING_END`.

Erreurs possibles pour `write`:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide);
* `EINVAL` (aucune startup sélectionnée; quantité de données insuffisante; données invalides);
* `EAGAIN` (période de traitement inachevée);
* `EPERM` (un investisseur-ange a choisi cette startup et son état ne peut plus être modifié).

L’opération de lecture termine une simulation (libérant les ressources associées) et lit son résultat. Ce résultat est retourné sous la forme d’une structure `startup_net_worth`:

	struct startup_net_worth {
		struct startup startup;
		unsigned reserved;
		long worth_by_year[16];
	};

Le champ `reserved` n'est présentement pas utilisé.

Le pilote vérifie que la longueur des données à lire soit exactement celle de cette structure.

Au terme de l’opération de lecture, l'entreprise actuelle est retirée du simulateur et aucune startup n’est sélectionnée pour les opérations subséquentes.

Erreurs possibles pour `read`:

* `EINTR` (opération interrompue par un signal, réessayez);
* `EFAULT` (pointeur invalide);
* `EINVAL` (aucune startup sélectionnée; quantité de données à lire pas égale à la taille de `startup_net_worth`; données invalides);
* `EAGAIN` (période de traitement inachevée);
* `EPERM` (un investisseur-ange a choisi cette startup et son état ne peut plus être modifié).

### SIGNGEL

Il se peut que le simulateur détecte que l’entreprise a tant de potentiel qu’un investisseur-ange s’y intéressera nécessairement. L’algorithme breveté menant à ce constat opère de façon asynchrone: par conséquent, il annonce ses résultats à l’aide du signal `SIGNGEL` (aussi appelé `SIGUSR1`).

Le signal est livré au premier programme ayant ouvert le fichier spécial `/dev/startup_simulator`. Si ce programme ferme le fichier, le signal sera livré au prochain programme qui l’ouvrira. Si aucun programme n’est disponible, le signal n’est évidemment pas livré. Par exemple, si le programme 188 ouvre le fichier, puis le programme 190 l’ouvre aussi, le signal sera livré à 188. Si 188 le ferme, le signal ne sera livré à aucun programme, même s’il est toujours ouvert dans 190. Si 201 ouvre le fichier par la suite, c’est lui qui recevra le signal.

Comme expliqué dans la section sur l’interface client, cette information doit être utilisée pour envoyer une réponse `ANGEL_INVESTOR` au client ayant créé l’entreprise et terminer la connexion.

Le nom de l’entreprise choisie par l’investisseur peut être récupéré à l’aide de l’ioctl `CSGAMES_GET_ANGELS_PICK`. L'entreprise ne peut plus être la cible d'opérations de lecture ou d'écriture à partir du moment où elle est choisie.

Notez que l’action par défaut du signal `SIGNGEL` est de terminer le programme qui le reçoit.

## VOTRE PAQUET CONTIENT

Ce paquet devrait contenir les fichiers suivants:

* readme.fr.pdf
* readme.en.pdf
* startup\_simulator.py
* csgames.c

Adressez-vous dès que possible à un responsable si des fichiers sont manquants.

### readme.fr.pdf

Ce document.

### readme.en.pdf

Ce document (en langue anglaise).

### startup_simulator.py

Un script Python que vous pouvez utiliser pour tester votre programme.

### csgames.c

Un squelette de programme C que vous pouvez utiliser pour démarrer le développement de votre application.

### poop.pdf

Un exemple d'excellent plan d'affaires que vous pouvez utiliser pour tester votre programme.

## AUSSI INSTALLÉ

Le programme `ngelctl` vous permet de contrôler certains aspects de la simulation de l'investisseur-ange. Quatre actions sont possibles:

* `ngelctl -d`: désactive la simulation de l'investisseur-ange
* `ngelctl -e`: active la simulation de l'investisseur-ange
* `ngelctl [-s] startup`: force la sélection de la startup nommée
* `ngelctl -c`: réinitialise la sélection de startup (libérant la startup du même coup)

Ce programme ne sera évidemment pas disponible dans l'environnement de correction.

## Grille de correction

La correction utilise deux critères principaux: l'exactitude des résultats et la performance du serveur. Les points d'exactitude sont attribués entièrement ou pas du tout, dépendant de si l'exigence est remplie. Les points de performance ne sont attribués que si l'exigence est remplie, et sont attribués au pro-rata du temps total d'exécution de la tâche vis-à-vis vos compétiteurs. Par exemple, si vous remplissez une exigence pour laquelle 10 points sont attribués pour la performance en 4.3 secondes, que l'équipe la plus rapide le fait en 3.5 secondes et que la plus lente le fait en 7.2 secondes, vous obtenez 10 - 10 * ((4.3-3.5)/(7.2-3.5)) = 7.84 points. De façon abstraite, la formule utilisée est `points - points * ((vous - min) / (max - min))`. (Si une seule équipe remplit une exigence ayant des points de performance, elle obtient tous les points de performance.)

Lors de la correction du traitement de plans d'affaires, la simulation de l'investisseur-ange sera **activée** pour vérifier l'exigence, puis **désactivée** lors du calcul de la performance.

| Exigence | ✔︎ | ⏱ |
|------------------|-------------------------:|---------------------------:|
|Le serveur peut recevoir deux connexions consécutives.|1|0|
|Le serveur peut accepter la création d'une startup.|1|0|
|Le serveur réagit correctement lorsque le pilote lui envoie le signal `SIGNGEL` et qu'une seule startup est simulée.|5|0|
|Le serveur réagit correctement lorsque le pilote lui envoie le signal `SIGNGEL` et que quatre startups sont simulées.|10|0|
|Le serveur peut traiter le plan d'affaires d'une entreprise.|5|15|
|Le serveur peut traiter le plan d'affaires de deux entreprises.|1|5|
|Le serveur peut traiter le plan d'affaires de trois entreprises.|1|0|
|Le serveur peut traiter le plan d'affaires de quatre entreprises.|1|5|
|Le serveur peut traiter le plan d'affaires de cinq entreprises.|1|0|
|Le serveur peut traiter le plan d'affaires de six entreprises.|1|0|
|Le serveur peut traiter le plan d'affaires de sept entreprises.|1|0|
|Le serveur peut traiter le plan d'affaires de huit entreprises.|1|5|
|Le serveur peut traiter le plan d'affaires de dix entreprises.|1|20|

De plus, 25 autre points seront attribués en fonction du comportement de votre serveur dans des cas d'erreur non-divulgués. Finalement, 5 points seront attribués en fonction de l'appréciation subjective de votre code source.

| Catégories | Points |
|:-----------|-------:|
|Exigences remplies|30|
|Performance|50|
|Comportement dans les cas limite|25|
|Style et élégance|5|
|**Total**|110|

## Aide et débogage

Voici une liste de fonctions qui pourraient vous être utiles:

* man 2 socket, bind, listen, accept
* man 2 open, flock, read, write, ioctl, close
* man 2 signal, kill, fork, waitpid
* man 2 gettimeofday
* man 3 htonl

Les outils suivants sont disponibles dans votre environnement de développement mais seront **désactivés** dans l'environnement de correction:

* dmesg
* ngelctl (voir plus haut)

### Pièges dans lesquels plusieurs tomberont et pour lesquels la correction sera sans pitié

* Assurez-vous que chaque valeur entière transmise par socket utilise l'ordre d'octets réseau.
* Tous les appels `ioctl` gérés par le simulateur peuvent échouer avec l'erreur `EINTR` et doivent seulement être réessayés.
* La plupart des appels à `read` et `write` peuvent échouer avec l'erreur `EINTR`, ou réussir seulement partiellement, et doivent être repris où ils ont arrêté.
