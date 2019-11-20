# TP1 synthèse

*Lafoix paul p1611978*

## Comment executer

La commande `premake4 gmake` génère les makefile dans le dossier *build/*. Ensuite, le script bash `makeTp1.sh` compile dans *bin/* et exécute le programme depuis la racine.

## Le code

### Fichier infos.h

Définit les structures Material_glsl et Light bien alignées pour être stockées dans des shader storage buffers.

### Classe Buffers

La classe buffers du fichier *buffersClass.h* permet de créer le buffer contenant les positions des vertex et les normales, et un buffer contenant les indices des matériaux pour chaque vertex. Les informations des matériaux sont stockées pour être rangées dans des shader storage buffers.

On différencie les objets statiques (sans animation = 1 seul fichier obj), des objets animés (un tableau de fichiers obj).

Si l'objet est animé, on ajoute deux vertex attrib pointer (1 pour les positions et 1 pour les normales) pour pointer sur la frame suivante. La fonction *setPointer* permet de décaler les layouts en fonction du numéro de la frame. Pour ne pas sortir du buffer pour la dernière keyframe, la première frame est dupliquée à la fin du buffer.

 
