from sqlalchemy import create_engine, MetaData
from sqlalchemy.orm import declarative_base
from sqlalchemy import Column, Integer, Float, DateTime
from datetime import datetime
from dotenv import load_dotenv
import os

# Carregar variáveis de ambiente do arquivo config.env
load_dotenv("config.env")
DATABASE_URL = os.getenv("DATABASE_URL")

# Configuração do SQLAlchemy
engine = create_engine(DATABASE_URL)
Base = declarative_base()
metadata = MetaData()

# Modelo para as leituras dos sensores
class SensorData(Base):
    __tablename__ = "sensor_data"
    
    id = Column(Integer, primary_key=True, index=True)
    sensor_1 = Column(Float, nullable=True)
    sensor_2 = Column(Float, nullable=True)
    sensor_3 = Column(Float, nullable=True)
    sensor_4 = Column(Float, nullable=True)
    sensor_5 = Column(Float, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)

# Função para apagar e recriar todas as tabelas
def reset_database():
    # Reflete as tabelas atuais no banco de dados
    metadata.reflect(bind=engine)
    # Apaga todas as tabelas
    metadata.drop_all(bind=engine)
    # Recria as tabelas
    Base.metadata.create_all(engine)

if __name__ == "__main__":
    print("Resetando o banco de dados...")
    reset_database()
    print("Banco de dados resetado e tabelas recriadas com sucesso.")