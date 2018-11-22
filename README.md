### Compilando (Linux)

Será necessário possuir a biblioteca SFML (ver [download](https://www.sfml-dev.org/download.php)) instalada em sua máquina.

Em ambientes debian (ex: Ubuntu) podes rodar o seguinte comando:
```sh
sudo apt-get install libsfml-dev
```

Por fim, execute:
```sh
# baixar dependências internas (ex: leitor de arquivos INI)
git submodule update --init --recursive
make
```

### Autores da Arte

- fonts/PCBius.ttf
	+ https://www.1001freefonts.com/pcbius.font

- sprites/\*_animation_\*.png
	+ https://craftpix.net/freebies/free-2d-fantasy-elf-warrior-character-sprites/



### Código Externo

- vendor/anim
	+ https://github.com/SFML/SFML/wiki/Source:-AnimatedSprite

- vendor/inih
	+ https://github.com/benhoyt/inih

- SFML:
	+ https://www.sfml-dev.org
