# SNES4ALL

Port de Snes9x (Super Nintendo) a Dreamcast, de Chui.

SNES4ALL es un emulador alternativo de Super Nintendo para Dreamcast, port de **Snes9x**, publicado por **Chui**. Este repositorio contiene su código fuente, descomprimido y con la historia de versiones.

> Archivo de preservación, no el desarrollo original. Los commits **no son de Chui**: cada uno reconstruye un paquete de fuentes publicado en su web.

## Contenido

- Raíz: `Makefile.dc`, `Makefile.linux`, `Makefile.win32`.
- `src/` (código), `data/` (recursos) y `doc/`.

## Características

- MMU para acceso directo a memoria y hardware PVR para dibujar los tiles de la SNES.
- Resolución escalada por hardware y núcleos de CPU en ensamblador.
- Autoframeskip, guardado de estado y SRAM, soporte de VMU y tarjeta SD.
- Menú (velocidad, frameskip, transparencias, sonido…) y gestor de ficheros con subdirectorios.
- 4 mandos emulados; selección 60-50 Hz en Dreamcast PAL.

## Historia de versiones

Un commit por paquete publicado, en orden cronológico (por la fecha de los ficheros de cada paquete), con una etiqueta por versión. Los commits llevan la fecha en que se crearon, no la de 2004-2011. El mensaje de cada commit incluye el SHA-256 del paquete original. Para comparar dos versiones: `git diff beta-1 beta-2`.

| Etiqueta | Versión | Paquete original | Fecha del paquete |
|---|---|---|---|
| `beta-1` | beta 1 | `snes4all-src-beta1.tar.bz2` | 2011-03-27 |
| `beta-2` | beta 2 | `snes4all-src-beta2.tar.bz2` | 2011-12-29 |

## Compilar

Para Dreamcast se compila con [KallistiOS](https://github.com/KallistiOS/KallistiOS) usando `Makefile.dc` (el ejecutable se llama `snes4all`); hay también Makefiles para Linux y Windows. **Este repositorio no ha compilado ninguna versión**: es un archivo del código publicado.

## Origen

- Web original: *Chui's DC projects* (`chui.dcemu.co.uk`), recuperada de Internet Archive. Copia de la web con todos los proyectos: [Chui-s-Dreamcast-Projects.](https://github.com/0skarzer0/Chui-s-Dreamcast-Projects.) ([ver la página de SNES4ALL](https://htmlpreview.github.io/?https://github.com/0skarzer0/Chui-s-Dreamcast-Projects./blob/main/SNES4ALL/Chuis_DC_projects_SNES4ALL.html)).
- Los paquetes originales (`*.tar.bz2`, `*.rar`) siguen en ese repositorio, en `SNES4ALL/archivos/`. Esta historia se extrajo de allí con `git filter-repo` (`SNES4ALL/src`); por eso los mensajes de commit mencionan rutas de ese repositorio.

## Créditos

- **Chui** – port a Dreamcast y autor de la web original (*Chui's DC projects*, `chui.dcemu.co.uk`).
- **Snes9x** (sus autores y colaboradores) – el emulador en el que se basa.
- **KallistiOS** – SDK de Dreamcast con el que se compila.
- **Internet Archive** (Wayback Machine) – de donde se recuperó la web.

## Licencia

Este repositorio no añade ninguna licencia. El código lleva la licencia de **Snes9x**: *freeware solo para uso personal*; el uso comercial, o cobrar por Snes9x o por software derivado, requiere pedir permiso a los titulares (aviso completo en los ficheros de `src/`). Algunos ficheros (`src/i386`) llevan GPL.
