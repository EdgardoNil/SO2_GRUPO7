import React from 'react';
import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import PieChartAndTable from './Dashboard';

function App() {
  return (
    <Router>
      <Routes>
        <Route exact path="/" element={<PieChartAndTable />} />
      </Routes>
    </Router>
  );
}

export default App;
