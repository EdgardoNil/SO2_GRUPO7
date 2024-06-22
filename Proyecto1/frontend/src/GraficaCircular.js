import React from 'react';
import { Chart as ChartJS, ArcElement, Tooltip, Legend } from 'chart.js';
import { Pie } from 'react-chartjs-2';

// Opcional: Registrar elementos de Chart.js
ChartJS.register(ArcElement, Tooltip, Legend);

const GraficaCircular = ({ chartData }) => {
  return <Pie data={chartData} />;
};

export default GraficaCircular;
