#!/usr/bin/env node
'use strict';

// Serves dist/ exactly like a static host would, under a sub-path that mimics
// GitHub Pages project sites: http://127.0.0.1:4000/cpp-patterns-trainer/

const path = require('path');
const express = require('express');

const PORT = Number(process.env.PORT) || 4000;
const BASE = '/cpp-patterns-trainer';
const app = express();
app.use(BASE, express.static(path.join(__dirname, '..', 'dist')));
app.get('/', (req, res) => res.redirect(`${BASE}/`));
app.listen(PORT, '127.0.0.1', () => console.log(`Static preview → http://127.0.0.1:${PORT}${BASE}/`));
