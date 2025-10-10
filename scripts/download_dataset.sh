#!/usr/bin/env bash
set -euo pipefail

# Configuración
DATA_DIR="./dataset/raw"
KAGGLE_DATASET="avdhesh15/books-processed-dataset"  # id del dataset en Kaggle
TMP_DIR="./dataset/tmp_download"
UNZIP=1   # 1 -> descomprimir si el archivo es zip

mkdir -p "${DATA_DIR}" "${TMP_DIR}"

echo "Descargando dataset desde Kaggle: ${KAGGLE_DATASET}"
# -p: path destino, --unzip: descomprime el zip si existe
kaggle datasets download -d "${KAGGLE_DATASET}" -p "${TMP_DIR}" --force

# Mover / descomprimir
# Si el dataset baja como un zip, el --unzip de la CLI lo habrá descomprimido
# Acomodamos cualquier csv o carpeta dentro de TMP_DIR hacia DATA_DIR
shopt -s nullglob
# mover todos csv y carpetas
for f in "${TMP_DIR}"/*; do
    echo "Moviendo: $f -> ${DATA_DIR}/"
    mv -v "$f" "${DATA_DIR}/"
done
shopt -u nullglob

# limpiar tmp
rmdir "${TMP_DIR}" 2>/dev/null || true

echo "Descarga e importación finalizadas. Archivos en: ${DATA_DIR}"
