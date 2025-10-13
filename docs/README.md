# PRÁCTICA 1 : COMUNICACIÓN ENTRE PROCESOS 

## Descripción general

Este programa implementa un sistema de busqueda eficiente sobre un conjunto de datos en formato csv, utilizando 
el dataset **books processed dataset** obtenido de Kaggle. Permite al usuario realizar consultas rápidas
mediante un sistema de indexación basado en una **Tabla Hash**, comunicando dos procesos no emparentados a tráves de **Tuberías Nombradas (FIFO)**.

## Campos del dataset

| Campo                   | Tipo de dato | Descripción |
|--------------------------|--------------|--------------|
| `title`                  | String       | Título del libro. |
| `author_name`            | String       | Nombre del autor o autores. |
| `image_url`              | String / URL | Enlace a la imagen de la portada del libro. |
| `num_pages`              | Entero       | Número total de páginas del libro. |
| `average_rating`         | Flotante     | Calificación promedio otorgada por los usuarios. |
| `text_review_count`      | Entero       | Número total de reseñas escritas por los usuarios. |
| `description`            | String       | Sinopsis o resumen del contenido del libro. |
| `5_star_rating_counts`   | Entero       | Cantidad de calificaciones de 5 estrellas. |
| `4_star_rating_counts`   | Entero       | Cantidad de calificaciones de 4 estrellas. |
| `3_star_rating_counts`   | Entero       | Cantidad de calificaciones de 3 estrellas. |
| `2_star_rating_counts`   | Entero       | Cantidad de calificaciones de 2 estrellas. |
| `1_star_rating_counts`   | Entero       | Cantidad de calificaciones de 1 estrella. |
| `total_rating_counts`    | Entero       | Total de calificaciones. |
| `genres`                 | String       | Géneros literarios asociados al libro. |

## Criterios de Búsqueda Implementados

Para esta práctica se utilizaron los campos *`title`* y *`author_name`* como criterios principales de búsqueda.

### 1. `title`
El título del libro es el campo mas intuitivo y directo para buscar en el dataset. Al ser una cadena de texto, facilita la generación del valor hash, además, la probabilidad de que dos libros compartan el mismo título es baja.

### 2. `author_name`
El nombre del autor permite agrupar libros relacionados y facilita la búsqueda entre obras de un mismo autor. Este sirve como segundo criterio en casos donde existan titulos similares o repetidos.

## Rangos de Valores

### 1. Titulo
Para la construcción de la tabla hash se utilizaron los **primeros 20 caracteres del titulo** de cada libro como clave principal de indexación, esto permite mantener una buena distribución dentro de la tabla.

Al realizar la consulta, el usuario puede ingresar cualquier cantidad de carácteres del título que desee buscar, el programa se encargará de calcular el valor hash correspondiente y localizar el registro mas cercano.

### 2. Autor
Para la construcción de la tabla hash se utilizaron los **primero 20 caracteres del nombre del autor** de cada libro como clave principal de indexacion, esto permite mantener una buena distribucion dentro de la tabla.

Al realizar la consulta. el usuario puede ingresar cualquier cantidad de carácteres del autor que desee buscar, el programa se encargará de calcular el valor hash correspondiente y localizar el registro más cercano. 

