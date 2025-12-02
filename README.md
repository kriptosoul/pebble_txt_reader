# Pebble TXT Reader - Full repo package (structure only)

This package contains the full project layout for compiling a Pebble TXT Reader in GitHub Actions using a self-hosted container image.
This archive does NOT include the large binary SDK/toolchain files for licensing and size reasons.

## What is included
- Dockerfile template in `docker/` (builds an image that contains the SDK and toolchain)
- `sdk/` folder (empty placeholder - you must populate it with the real Pebble SDK)
- `toolchain/` folder (empty placeholder - you must populate it with the real ARM toolchain)
- GitHub Actions workflow configured to use your uploaded GHCR image `ghcr.io/<your-username>/pebble-sdk:latest`
- `src/main.c` - Reader app source with pagination, font size controls, dark mode and persistence
- `resources/data/book.txt` - sample book to test locally
- `appinfo.json`, `wscript`

## Required downloads (fill the placeholders)
Download the actual SDK and toolchain and place them into the `sdk/` and `toolchain/` folders respectively.

### Pebble SDK (example mirror)
Look for a Rebble or archived Pebble SDK distribution; once extracted, place the SDK root folder inside `sdk/`.

### GNU ARM toolchain (example official ARM link)
Official ARM link (may be removed):
https://developer.arm.com/-/media/Files/downloads/gnu-rm/4_9-2015q3/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2

## How to prepare the container image and push to GHCR (one-time)
1. Populate `sdk/` with the extracted contents of the PebbleSDK (so `/opt/pebble-sdk` inside the container).
2. Populate `toolchain/` with extracted `gcc-arm-none-eabi` toolchain (so `/opt/arm-none-eabi` inside the container).
3. From repo root (locally) build the image:
   ```bash
   docker build -t ghcr.io/your-username/pebble-sdk:latest docker/
   ```
4. Push image to GHCR:
   ```bash
   echo $CR_PAT | docker login ghcr.io -u your-username --password-stdin
   docker push ghcr.io/your-username/pebble-sdk:latest
   ```
5. Update `.github/workflows/build.yml` container.image value if you chose a different name.

## How to test locally with Docker
1. Build image (see above)
2. Run interactive container:
   ```bash
   docker run --rm -v $(pwd):/project -w /project -it ghcr.io/your-username/pebble-sdk:latest /bin/bash
   ```
3. Inside container run `pebble build`

