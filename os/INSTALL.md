# FICHIER PRIVÉ

Seulement utile pour les tests.

# Comment charger startup_simulator.ko

Solution temporaire (ne se charge pas tout seul au démarrage de la machine):

    cd private
    make
    sudo cp startup_simulator.rules /lib/udev/rules.d
    sudo insmod startup_simulator.ko

Cette solution ne laisse pas d'autres traces que /lib/udev/rules.d/startup_simulator.rules.

Solution "permanente" (se charge tout seul au démarrage de la machine):

    cd private
    make
    sudo make install
    sudo shutdown -r now # redémarre immédiatement

L'installation laisse les traces suivantes:

* /lib/modules/$(shell uname -r)/kernel/drivers/char/startup_simulator.ko
* /lib/udev/rules.d/startup_simulator.rules
* Ligne "startup_simulator" dans /etc/modules

Une fois tous ces éléments supprimés, exécutez `sudo depmod -a`.
