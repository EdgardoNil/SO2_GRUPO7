import React, { useEffect, useState } from 'react';
import GraficaCircular from './GraficaCircular'; // Asegúrate de importar correctamente GraficaCircular desde su ubicación real
import Navbar from './Navbar';
import './Dashboard.css'; // Asegúrate de importar tu archivo CSS aquí

const PieChartAndTable = () => {
  const [data, setData] = useState([]);
  const [dataPie, setDataPie] = useState([]);
  const [chartCPU, setChartCPU] = useState(null);
  const [ultimaData, setUltimaData] = useState([]);

  const memoriaTotal = 4064878592;

  const fetchData = async () => {
    try {
      const response = await fetch('http://127.0.0.1:5000/datos');
      const data = await response.json();
      console.log('Datos recibidos del backend:', data);
      setData(data);
      fetchDataPie();
      fetchUltimaData();
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  };

  const fetchDataPie = async () => {
    try {
      const response = await fetch('http://127.0.0.1:5000/datagrafica');
      const data = await response.json();
      console.log('Datos recibidos del backend (gráfica):', data);
      setDataPie(data);

      const primerosDiez = data.slice(0, 10);
      const labels = [];
      const valores = [];

      primerosDiez.forEach((elemento) => {
        if (elemento.porcentaje > 0) {
          labels.push(elemento.proceso);
          valores.push(elemento.porcentaje);
        }
      });

      const penultimoIndice = data.length - 2;
      const penultimoElemento = data[penultimoIndice];
      if (penultimoElemento.porcentaje > 0) {
        labels.push(penultimoElemento.proceso);
        valores.push(penultimoElemento.porcentaje);
      }

      const chart_cpu = {
        labels,
        datasets: [
          {
            data: valores,
            backgroundColor: [
              'rgba(142, 68, 173, 0.3)',
              'rgba(255, 99, 132, 0.3)',
              'rgba(54, 162, 235, 0.3)',
              'rgba(23, 165, 137, 0.3)',
              'rgba(241, 196, 15, 0.3)',
              'rgba(82, 190, 128, 0.3)',
              'rgba(148, 49, 38, 0.3)',
              'rgba(240, 243, 244, 0.3)',
              'rgba(237, 187, 153, 0.3)',
              'rgba(133, 193, 233, 0.3)',
              'rgba(11, 83, 69, 0.3)',
            ],
            borderColor: [
              'rgba(142, 68, 173, 1)',
              'rgba(255, 99, 132, 1)',
              'rgba(54, 162, 235, 1)',
              'rgba(23, 165, 137, 1)',
              'rgba(241, 196, 15, 1)',
              'rgba(82, 190, 128, 1)',
              'rgba(148, 49, 38, 1)',
              'rgba(240, 243, 244, 1)',
              'rgba(237, 187, 153, 1)',
              'rgba(133, 193, 233, 1)',
              'rgba(11, 83, 69, 1)',
            ],
            borderWidth: 1,
          },
        ],
      };
      setChartCPU(chart_cpu);
    } catch (error) {
      console.error('Error fetching data (gráfica):', error);
    }
  };

  const fetchUltimaData = async () => {
    try {
      const response = await fetch('http://127.0.0.1:5000/ultimadata');
      const data = await response.json();
      console.log('Última data recibida del backend:', data);
      setUltimaData(data);
    } catch (error) {
      console.error('Error fetching ultima data:', error);
    }
  };

  useEffect(() => {
    const fetchDataInterval = setInterval(() => {
      fetchData();
    }, 3000);

    fetchData(); // Llama a la función inmediatamente al cargar el componente

    return () => {
      clearInterval(fetchDataInterval); // Limpia el intervalo cuando el componente se desmonta para evitar fugas de memoria
    };
  }, []);

  return (
    <>
      <Navbar />
      <div className="dashboard-container">
        <h1 className="dashboard-title">Dashboard</h1>

        <div className="dashboard-content">
          <div className="chart-container">
            {chartCPU ? <GraficaCircular chartData={chartCPU} /> : 'Cargando...'}
          </div>

          <div className="table-container">
            <div className="scrollable-table">
              <table>
                <thead>
                  <tr>
                    <th>PID</th>
                    <th>Nombre</th>
                    <th colSpan="2">Memoria</th>
                  </tr>
                </thead>
                <tbody>
                  {dataPie.map((item, index) => (
                    <tr key={index}>
                      <td>{item.pid}</td>
                      <td>{item.proceso}</td>
                      <td>{(item.memoria_usada / 1024 / 1024).toFixed(2)} MB</td>
                      <td>
                        {(item.memoria_usada / memoriaTotal * 100).toFixed(2)} %
                        <div className="memory-bar">
                          <div className="memory-bar-fill" style={{ width: `${(item.memoria_usada / memoriaTotal * 100).toFixed(2)}%` }}></div>
                        </div>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        </div>

        <h2>Solicitudes</h2>
        <div className="dashboard-content">
          <div className="table-container-two">
            <table>
              <thead>
                <tr>
                  <th>PID</th>
                  <th>Proceso</th>
                  <th>Llamada</th>
                  <th>Tamaño</th>
                  <th>Fecha y Hora</th>
                </tr>
              </thead>
              <tbody>
                {ultimaData.map((entry, index) => (
                  <tr key={index}>
                    <td>{entry.pid}</td>
                    <td>{entry.proceso}</td>
                    <td>{entry.llamada}</td>
                    <td>{(entry.tamano / 1024 / 1024).toFixed(2)} MB</td>
                    <td>{entry.fecha_hora}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </>
  );
};

export default PieChartAndTable;
