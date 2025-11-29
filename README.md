# Pebble TXT Reader

App para Pebble Time y demás modelos (basalt/chalk/aplite) que permite leer archivos `.txt` en el reloj.

**Características**:
- Enviar un `.txt` desde la página de configuración del companion (pegar o subir).
- Navegar páginas con los botones **UP** (atrás) y **DOWN** (adelante).
- Pulsación corta en **SELECT** cambia el tamaño de la fuente (pequeña/mediana/grande).
- Pulsación larga en **SELECT** reinicia la lectura al inicio del libro.
- Guarda el texto y la última página en persistencia para retomar tras salir.

## Archivos importantes
- `appinfo.json`
- `src/main.c`
- `pebble-js-app.js`
- `config.html`
- `.github/workflows/build.yml` (para compilar en GitHub Actions)

## Cómo compilar (local)
Necesitas el Pebble SDK / Rebble toolchain instalado. En la raíz del proyecto ejecuta:
```
pebble build
```

## Compilación automática en GitHub Actions
El proyecto incluye un workflow en `.github/workflows/build.yml` que instala el SDK y compila el `.pbw`. Tras un push a `main` el workflow deja el `.pbw` como artifact descargable.