# Programme de génération d'histogrammes CO2

Ce programme est conçu pour télécharger des données sur les émissions de CO2 territoriales par secteur en France à partir de l'API OpenDataSoft, puis générer des histogrammes CO2 pour chaque année à partir de ces données.

## Méthodologie utilisée

Le programme suit les étapes suivantes :

1. Téléchargement des données JSON à partir de l'API OpenDataSoft.
2. Analyse des données JSON pour extraire les années, les sous-secteurs et les taux d'émissions de CO2.
3. Calcul des valeurs minimales et maximales des taux de CO2.
4. Regroupement des données par année et sous-secteur.
5. Génération d'histogrammes CO2 pour chaque année avec des sous-secteurs distincts.
6. Les histogrammes sont générés au format PNG et sauvegardés dans le dossier "graphique".

## Objectif du projet

L'objectif de ce projet est de visualiser les émissions de CO2 territoriales par secteur en France au fil des années à l'aide d'histogrammes. Cela permet de mieux comprendre la répartition des émissions de CO2 par secteur et d'identifier les tendances au fil du temps.

## Bibliothèques utilisées et comment les télécharger

Ce programme utilise plusieurs bibliothèques externes, dont voici la liste :

- [gd](https://libgd.github.io/): Bibliothèque pour la manipulation d'images. Vous pouvez l'installer avec la commande suivante sur Debian/Ubuntu :
```
sudo apt-get install libgd-dev
```

- [FreeType](https://www.freetype.org/): Bibliothèque pour la gestion de polices. Vous pouvez l'installer avec la commande suivante sur Debian/Ubuntu :
```
sudo apt-get install libfreetype6-dev
```
- [nlohmann/json](https://github.com/nlohmann/json): Bibliothèque JSON en C++. Vous pouvez l'installer à l'aide de gestionnaires de dépendances comme [vcpkg](https://github.com/microsoft/vcpkg) ou [conan](https://conan.io/).

- [libcurl](https://curl.se/libcurl/): Bibliothèque pour effectuer des requêtes HTTP. Vous pouvez l'installer avec la commande suivante sur Debian/Ubuntu :
```
sudo apt-get install libcurl4-openssl-dev
```


## Compilation du programme sur Debian/Ubuntu

Pour compiler le programme, vous pouvez utiliser un compilateur C++ tel que g++ avec la commande suivante :

```bash
g++ -o emissionCO2 main.cpp -lgd -lfreetype -lcurl -lnlohmann_json
```

Assurez-vous d'être dans le répertoire contenant le fichier source main.cpp et que les bibliothèques requises sont installées

# Configuration de Buildroot et QEMU pour ce projet

Ce projet comprend une configuration spécifique pour Buildroot et QEMU pour faciliter le déploiement et l'exécution du programme sur un environnement de développement ou une cible matérielle.

## Configuration de Buildroot

1. Téléchargez et installez Buildroot depuis le [site officiel de Buildroot](https://buildroot.org/).

2. Clonez ce dépôt Git contenant la configuration spécifique pour Buildroot :

    ```bash
    git clone https://github.com/votre-utilisateur/projet-buildroot-config.git
    ```

3. Copiez le fichier `.config` de la configuration spécifique dans le répertoire principal de Buildroot :

    ```bash
    cp projet-buildroot-config/buildroot-config/.config buildroot/.config
    ```

4. Exécutez la commande `make menuconfig` pour personnaliser davantage la configuration de Buildroot selon vos besoins.

5. Compilez Buildroot en exécutant :

    ```bash
    make
    ```

## Configuration de QEMU

1. Installez QEMU sur votre système si ce n'est pas déjà fait. Sur Debian/Ubuntu, vous pouvez utiliser la commande suivante :

    ```bash
    sudo apt-get install qemu-system
    ```

2. Démarrez QEMU avec l'image générée par Buildroot :

    ```bash
    qemu-system-arm -M versatilepb -kernel output/images/zImage -dtb output/images/versatile-pb.dtb -drive file=output/images/rootfs.ext2,if=scsi -append "root=/dev/sda console=ttyAMA0" -serial stdio
    ```

3. Assurez-vous que les dépendances nécessaires sont également présentes sur la cible Buildroot.

## Transfert et exécution du programme sur Buildroot

Une fois que vous avez configuré Buildroot et généré une image, vous pouvez transférer le programme vers la cible Buildroot (par exemple, un émulateur QEMU ou un matériel embarqué) en utilisant des méthodes telles que SCP, FTP ou en les incluant dans l'image de la racine du système de fichiers.

Assurez-vous que le chemin d'accès au programme dans l'image Buildroot correspond à l'emplacement attendu par le programme.

Pour exécuter le programme, accédez à la cible Buildroot et exécutez-le :

```bash
./emissionCO2
```

Le programme téléchargera les données, générera les histogrammes et les sauvegardera dans le dossier "graphique" de votre système Buildroot.

N'oubliez pas d'adapter les sections "Configuration de Buildroot" et "Configuration de QEMU" en fonction de votre configuration spécifique. Assurez-vous d'inclure les fichiers de configuration et les paramètres appropriés pour Buildroot et QEMU.



