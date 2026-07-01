-- ============================================================
-- PC5 - Indice Espacial R-Tree (via GiST) en PostgreSQL
--
-- Desde PG 8.2 no existe un access method "rtree" nativo: la
-- estructura R-Tree vive dentro de GiST (Generalized Search Tree).
-- Indexa bounding boxes que se superponen -> contencion,
-- interseccion y cercania.
-- Sirve para: <@, @>, &&, ~=, <-> (distancia / KNN).
-- (Es el equivalente multidimensional del B-Tree: arbol de
-- bounding boxes en vez de claves ordenadas.)
-- ============================================================
DROP TABLE IF EXISTS lugares;
CREATE TABLE lugares (
    id        SERIAL PRIMARY KEY,
    nombre    VARCHAR(50) NOT NULL,
    ubicacion POINT       NOT NULL
);
-- Volumen real: 100 000 puntos en un plano de 0..1000 x 0..1000.
-- Sin volumen el planner prefiere Seq Scan y el indice "no se nota".
INSERT INTO lugares (nombre, ubicacion)
SELECT 'lugar_' || g,
       POINT((random() * 1000)::numeric(10,2), (random() * 1000)::numeric(10,2))
FROM generate_series(1, 100000) AS g;
ANALYZE lugares;
-- ------------------------------------------------------------
-- (1) SIN indice -> Seq Scan: contencion en un area (bounding box)
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM lugares WHERE ubicacion <@ BOX '((400,400),(600,600))';
-- Creamos el indice GiST (R-Tree de bounding boxes)
CREATE INDEX idx_lugares_ubicacion ON lugares USING gist (ubicacion);
ANALYZE lugares;
-- ------------------------------------------------------------
-- (2) CON indice -> Index Scan GiST en la contencion
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM lugares WHERE ubicacion <@ BOX '((400,400),(600,600))';
-- ------------------------------------------------------------
-- (3) KNN: los 10 lugares mas cercanos a un punto (mismo indice GiST)
-- ------------------------------------------------------------
EXPLAIN (ANALYZE, BUFFERS)
SELECT id, nombre, ubicacion, ubicacion <-> POINT(500,500) AS distancia
FROM lugares
ORDER BY ubicacion <-> POINT(500,500)
LIMIT 10;
-- Indices existentes sobre la tabla
SELECT indexname, indexdef FROM pg_indexes WHERE tablename = 'lugares';