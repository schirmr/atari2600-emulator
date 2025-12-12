#!/bin/bash

echo "======================================="
echo "  Instalador de dependências do projeto"
echo "======================================="

if [ "$EUID" -ne 0 ]; then
    echo "Por favor, execute como root (use: sudo ./requirements.sh)"
    exit 1
fi

echo
echo "➡️  Atualizando lista de pacotes..."
apt update -y

echo
echo "➡️  Instalando build-essential..."
apt install -y build-essential

echo
echo "✔️  Instalação concluída!"
echo "✔️  Dependências essenciais instaladas com sucesso."
echo "======================================="
