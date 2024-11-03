#!/bin/bash

# Defina variáveis
PROJECT_DIR="/opt/sensor_data_api" # Substitua pelo caminho real do seu projeto
SERVICE_NAME="sensor_data_api.service"
USER="$(whoami)"

# 1. Criar o diretório do projeto se não existir
if [ ! -d "$PROJECT_DIR" ]; then
    echo "Criando diretório do projeto em $PROJECT_DIR"
    mkdir -p "$PROJECT_DIR"
fi

# 2. Instalar Python e pip se não estiverem instalados
sudo dnf install -y python3 python3-pip python3-venv

# 3. Criar e ativar o ambiente virtual
python3 -m venv "$PROJECT_DIR/venv"
source "$PROJECT_DIR/venv/bin/activate"

# 4. Instalar dependências do FastAPI
pip install fastapi uvicorn sqlalchemy python-dotenv pydantic

# 5. Criar script de inicialização
cat <<EOL > "$PROJECT_DIR/start_app.sh"
#!/bin/bash
source "$PROJECT_DIR/venv/bin/activate"
exec uvicorn main:app --host 0.0.0.0 --port 8000 --reload
EOL

chmod +x "$PROJECT_DIR/start_app.sh"

# 6. Criar arquivo de serviço do systemd
cat <<EOL | sudo tee /etc/systemd/system/$SERVICE_NAME
[Unit]
Description=FastAPI Application
After=network.target

[Service]
User=$USER
WorkingDirectory=$PROJECT_DIR
ExecStart=$PROJECT_DIR/start_app.sh
Restart=always

[Install]
WantedBy=multi-user.target
EOL

# 7. Recarregar systemd, habilitar e iniciar o serviço
sudo systemctl daemon-reload
sudo systemctl enable $SERVICE_NAME
sudo systemctl start $SERVICE_NAME

# 8. Verificar status do serviço
sudo systemctl status $SERVICE_NAME

echo "Configuração concluída! Acesse o aplicativo em http://<seu-ip>:8000"