from fastapi import FastAPI, Depends, HTTPException
from sqlalchemy import create_engine, Column, Integer, Float, DateTime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session
from dotenv import load_dotenv
from datetime import datetime, timedelta
from pydantic import BaseModel
from typing import Optional, List
import os

# Carregar variáveis de ambiente
load_dotenv("config.env")
DATABASE_URL = os.getenv("DATABASE_URL")

# Configuração do SQLAlchemy
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# Modelo de dados para os sensores
class SensorData(Base):
    __tablename__ = "sensor_data"

    id = Column(Integer, primary_key=True, index=True)
    sensor_1 = Column(Float, nullable=True)
    sensor_2 = Column(Float, nullable=True)
    sensor_3 = Column(Float, nullable=True)
    sensor_4 = Column(Float, nullable=True)
    sensor_5 = Column(Float, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)

# Criar as tabelas no banco de dados
Base.metadata.create_all(bind=engine)

# Inicializar o FastAPI
app = FastAPI()

# Dependência para a sessão do banco de dados
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

# Modelo Pydantic para a resposta
class SensorDataBase(BaseModel):
    id: int
    sensor_1: Optional[float] = None
    sensor_2: Optional[float] = None
    sensor_3: Optional[float] = None
    sensor_4: Optional[float] = None
    sensor_5: Optional[float] = None
    created_at: datetime

    class Config:
        from_attributes = True  # Mudança aqui

# Rota para inserir dados dos sensores
@app.post("/data")
def insert_sensor_data(data: dict, db: Session = Depends(get_db)):
    filtered_data = {k: v for k, v in data.items() if v is not None and k.startswith("sensor")}

    if not filtered_data:
        raise HTTPException(status_code=400, detail="Nenhum dado de sensor válido fornecido.")

    sensor_data = SensorData(**filtered_data)
    db.add(sensor_data)
    db.commit()
    db.refresh(sensor_data)
    return {"message": "Dados inseridos com sucesso", "id": sensor_data.id}

# Rota para obter todos os dados dos sensores
@app.get("/data", response_model=List[SensorDataBase])
def get_sensor_data(db: Session = Depends(get_db)):
    return db.query(SensorData).all()

# Rota para obter dados de um sensor específico
@app.get("/data/sensor/{sensor_id}")
def get_sensor_data_by_id(sensor_id: int, db: Session = Depends(get_db)):
    sensor_data = db.query(SensorData).filter(getattr(SensorData, f'sensor_{sensor_id}').isnot(None)).all()

    if not sensor_data:
        raise HTTPException(status_code=404, detail="Nenhum dado encontrado para o sensor solicitado.")

    result = []
    for record in sensor_data:
        record_dict = {
            "id": record.id,
            f"sensor_{sensor_id}": getattr(record, f'sensor_{sensor_id}'),
            "created_at": record.created_at
        }
        result.append(record_dict)

    return result

# Rota para buscar dados de sensores em um determinado período
@app.get("/data/sensor/{sensor_number}/period")
def get_sensor_data_period(sensor_number: int, start_date: str, end_date: str, db: Session = Depends(get_db)):
    # Verifica se o número do sensor está no intervalo válido
    if sensor_number < 1 or sensor_number > 5:
        raise HTTPException(status_code=400, detail="O número do sensor deve estar entre 1 e 5.")

    start_datetime = datetime.strptime(start_date, "%Y-%m-%d")
    end_datetime = datetime.strptime(end_date, "%Y-%m-%d") + timedelta(days=1)

    # Escolher o atributo do sensor dinamicamente
    sensor_column = f'sensor_{sensor_number}'

    # Filtrar dados que não são nulos e dentro do intervalo de datas
    sensor_data = db.query(SensorData).filter(
        SensorData.created_at >= start_datetime,
        SensorData.created_at < end_datetime,
        getattr(SensorData, sensor_column).isnot(None)  # Adiciona a condição para verificar se o valor não é nulo
    ).all()

    return [
        {
            "id": data.id,
            f"sensor_{sensor_number}": getattr(data, sensor_column),  # Usando f-string para formatar a chave
            "created_at": data.created_at
        } 
        for data in sensor_data
    ]

# Rota para atualizar dados de um registro específico
@app.put("/data/{data_id}")
def update_sensor_data(data_id: int, data: dict, db: Session = Depends(get_db)):
    sensor_data = db.query(SensorData).filter(SensorData.id == data_id).first()

    if not sensor_data:
        raise HTTPException(status_code=404, detail="Registro não encontrado.")

    for key, value in data.items():
        if value is not None and key.startswith("sensor"):
            setattr(sensor_data, key, value)

    db.commit()
    return {"message": "Dados atualizados com sucesso"}

# Rota para deletar um registro específico
@app.delete("/data/{data_id}")
def delete_sensor_data(data_id: int, db: Session = Depends(get_db)):
    sensor_data = db.query(SensorData).filter(SensorData.id == data_id).first()

    if not sensor_data:
        raise HTTPException(status_code=404, detail="Registro não encontrado.")

    db.delete(sensor_data)
    db.commit()
    return {"message": "Registro deletado com sucesso"}