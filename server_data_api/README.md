# API de Monitoramento de Temperatura

## Descrição

Este projeto consiste em uma API construída com FastAPI que coleta dados de sensores de temperatura e os armazena em um banco de dados PostgreSQL. A API permite criar, ler, atualizar e excluir leituras de temperatura, além de consultar dados por diferentes intervalos de tempo.

## Estrutura do Projeto

```
/opt/sensor_data_api/
├── credential.env          # credenciais do DB
├── main.py                 # Arquivo principal da API
```

## Dependências
1. FastAPI: Framework para construir APIs em Python.
```
pip install fastapi
```

2. Uvicorn: Servidor ASGI para executar a aplicação FastAPI.
```
pip install uvicorn
```

3. SQLAlchemy: ORM para interagir com o banco de dados.
```
pip install sqlalchemy
```

4. psycopg2: Adaptador PostgreSQL para Python.
```
pip install psycopg2
```

5. python-dotenv: Para ler variáveis de ambiente de arquivos .env.
```
pip install python-dotenv
```

## Arquivo credential.env
Formato do arquivo credential.env:
```
DB_USER=user
DB_PASSWORD=password
DB_NAME=db_name
DB_HOST=localhost
```


## Rotas da API
| Método | Rota                            | Descrição                                   |
|--------|---------------------------------|---------------------------------------------|
| POST   | `/data`                         | Insere dados dos sensores                   |
| GET    | `/data`                         | Retorna todos os dados dos sensores         |
| GET    | `/data/sensor/{sensor_id}`     | Retorna dados de um sensor específico       |
| GET    | `/data/sensor/{sensor_number}/period` | Retorna dados de um sensor em um período específico |
| PUT    | `/data/{data_id}`              | Atualiza dados de um registro específico     |
| DELETE | `/data/{data_id}`              | Deleta um registro específico                |


## Documentação da Rota: 
/data/sensor/{sensor_number}/period
### Descrição
Esta rota permite recuperar dados de um sensor específico em um intervalo de datas definido. Os dados retornados serão apenas aqueles que possuem valores válidos (não nulos) para o sensor solicitado.

### Método HTTP
GET
Estrutura da Rota
```
/data/sensor/{sensor_number}/period
```

### Parâmetros de Rota
{sensor_number}: O número do sensor para o qual você deseja recuperar os dados. Este número deve estar entre 1 e 5.
### Parâmetros de Consulta
Você deve incluir os seguintes parâmetros na consulta:

start_date: Data de início do intervalo. Deve ser fornecida no formato YYYY-MM-DD.
end_date: Data de término do intervalo. Também deve ser fornecida no formato YYYY-MM-DD.
#### Exemplo de Uso
1. Recuperar Dados do Sensor 1
Para buscar os dados do Sensor 1 entre 1º de novembro de 2024 e 2 de novembro de 2024, a requisição seria:

```
GET /data/sensor/1/period?start_date=2024-11-01&end_date=2024-11-02
```