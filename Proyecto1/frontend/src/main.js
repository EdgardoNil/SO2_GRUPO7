import React from 'react';
import ReactDOM from 'react-dom';
import App from './App'; // Ajusta la ruta según la ubicación real de tu componente App
import './index.css'; // Importa tu archivo CSS aquí si es necesario

ReactDOM.render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
  document.getElementById('root')
);
