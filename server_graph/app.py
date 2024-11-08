import streamlit as st
import requests
import plotly.graph_objects as go
from datetime import datetime

# Configurações
API_URL = "http://192.168.0.3:8000/data/sensor/{sensor_number}/period"

def get_sensor_data(sensor_number, start_date, end_date):
    """Consulta a API para obter os dados de um sensor no período especificado"""
    params = {"start_date": start_date, "end_date": end_date}
    try:
        response = requests.get(API_URL.format(sensor_number=sensor_number), params=params)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        st.error(f"Erro ao consultar a API: {e}")
        return []

def plot_sensor_data(sensor_data, sensor_number):
    """Plota um gráfico para o sensor especificado"""
    # Filtra os dados do sensor
    timestamps = [datetime.fromisoformat(item["created_at"]) for item in sensor_data]
    values = [item[f"sensor_{sensor_number}"] for item in sensor_data]

    fig = go.Figure()
    fig.add_trace(go.Scatter(x=timestamps, y=values, mode='lines+markers', name=f'Sensor {sensor_number}'))
    fig.update_layout(
        title=f'Temperatura - Sensor {sensor_number}',
        xaxis_title='Data e Hora',
        yaxis_title='Temperatura (°C)',
        xaxis=dict(type='date', tickformat='%Y-%m-%d %H:%M')
    )
    st.plotly_chart(fig, use_container_width=True)

def main():
    st.title("Monitoramento de Temperatura dos Sensores")
    
    # Selecionar período de exibição
    st.sidebar.header("Selecionar Período")
    start_date = st.sidebar.date_input("Data de Início", datetime.now().date())
    end_date = st.sidebar.date_input("Data de Fim", datetime.now().date())
    
    # Validar o período
    if start_date > end_date:
        st.error("A data de início não pode ser posterior à data de fim.")
        return

    start_date_str = start_date.strftime("%Y-%m-%d")
    end_date_str = end_date.strftime("%Y-%m-%d")

    # Exibir gráficos para cada sensor
    for i in range(1, 6):
        st.header(f"Gráfico de Temperatura do Sensor {i}")
        sensor_data = get_sensor_data(i, start_date_str, end_date_str)
        if sensor_data:
            plot_sensor_data(sensor_data, i)
        else:
            st.write(f"Sem dados para o Sensor {i} no período selecionado.")

if __name__ == "__main__":
    main()
