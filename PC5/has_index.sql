-- ============================================================
-- PC5 - Indice Hash en PostgreSQL
--
-- Hash = aplica una funcion hash a la clave y reparte las filas
-- en buckets -> busqueda por igualdad O(1) promedio.
-- Sirve SOLO para: =. NO soporta rangos ni ORDER BY.
-- Desde PG10 es crash-safe (se registra en el WAL).
-- (Es la misma idea de la HashTable de la PC4: funcion hash +
-- manejo de colisiones, igualdad O(1).)
-- ============================================================
DROP TABLE IF EXISTS sesiones;
CREATE TABLE sesiones (
    id      SERIAL PRIMARY KEY,
    token   VARCHAR(32) NOT NULL,
    usuario VARCHAR(50) NOT NULL,
    creado  DATE        NOT NULL
);
-- Volumen real: 100 000 filas, token unico tipo md5.
-- Sin volumen el planner prefiere Seq Scan y el indice "no se nota".
INSERT INTO sesiones (token, usuario, creado)
SELECT md5(g::text),
       'user_' || (g % 5000),
       DATE '2026-01-01' + (random() * 365)::int
FROM generate_series(1, 100000) AS g;
ANALYZE sesiones;
-- Token real que se usara en las pruebas de igualdad
SELECT token FROM sesiones WHERE id = 42500 \gset
-- ------------------------------------------------------------
-- (1) SIN indice -> Seq Scan: recorre las 100 000 filas
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM sesiones WHERE token = :'token';
-- Creamos el indice Hash
CREATE INDEX idx_ses_token ON sesiones USING hash (token);
ANALYZE sesiones;
-- ------------------------------------------------------------
-- (2) CON indice -> Index Scan usando el hash (igualdad exacta)
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM sesiones WHERE token = :'token';
-- ------------------------------------------------------------
-- (3) El indice Hash NO sirve para rangos: se ignora y vuelve a Seq Scan
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM sesiones WHERE token > '80000000000000000000000000000000';
-- ------------------------------------------------------------
-- (4) Tampoco ayuda a ordenar
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM sesiones ORDER BY token LIMIT 10;
-- Indices existentes sobre la tabla
SELECT indexname, indexdef FROM pg_indexes WHERE tablename = 'sesiones';