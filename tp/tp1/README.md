# TP1 synthèse

*Lafoix paul p1611978*

## Comment executer

La commande `premake4 gmake` génère les makefile dans le dossier **build/**. Ensuite, le script bash `makeTp1.sh` compile dans **bin/** et exécute le programme depuis la racine.

## Le code

### Fichier infos.h

Définit les structures Material_glsl et Light bien alignées pour être stockées dans des shader storage buffers.

### Classe Buffers

La classe buffers du fichier **buffersClass.h** permet de créer le buffer contenant les positions des vertex et les normales, et un buffer contenant les indices des matériaux pour chaque vertex (qu'on duplique en 3 pour chaque vertex des triangles). Les informations des matériaux sont stockées pour être rangées dans des shader storage buffers.

On différencie les objets statiques (sans animation = 1 seul fichier obj), des objets animés (un tableau de fichiers obj).

Si l'objet est animé, on ajoute deux vertex attrib pointer (1 pour les positions et 1 pour les normales) pour pointer sur la frame suivante. La fonction *setPointer* permet de décaler les layouts en fonction du numéro de la frame. Pour ne pas sortir du buffer pour la dernière keyframe, la première frame est dupliquée à la fin du buffer.

### Classe renderer dérivée de App

C'est la classe qui contient toute la scène : les objets statiques et animés, les lumières, les buffer pour le shadow mapping...

La scène est composée d'une petite ville et d'un robot animé (controlable avec Z,Q,S,D). Le soleil tourne autour de la scène au fil du temps, et lorsqu'il fait nuit, les lampadaires s'allument (progressivement, à voir dans la fonction **updateModels**).

La brdf est.....

Lors du render, on génère autant de shadow map qu'il y a de lumières.

### Améliorations à faire

Factoriser le code d'affichage des objets statiques et animés.

Gérer plusieurs shadowmap pour différentes lumières génériquement. Pour l'instant, il faut 1 depth_buffer par lumière, donc 1 texture par lumière, ce qui est vite contraignant... On doit alors dans le shader faire des sous cas en fonction de la lumière, ce qui est très peu performant.

Améliorer la qualité des ombres, qui sont pour l'instant pixelisées.