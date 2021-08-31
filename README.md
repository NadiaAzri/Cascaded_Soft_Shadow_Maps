# Computer Graphics
## Cascaded Soft Shadow Maps Based on PCF Filtering

![result](Images/3_16.png) 

Physically accurate soft shadows in real-time applications can be imitated by subdividing the area light source into multiple samples and creating a shadow map for each sample and then accumulating them. 

In this work, we fitted in this context, and we proposed a new technique to reduce the rendering time required to achieve a high quality of soft shadows in real-time rendering. We generated physically accurate soft shadows using the Cascaded Shadow Maps technique (CSMs) to reduce further the samples of the area light source on far cascades, and use multiple implemented PCF filters on the GPU which allowed us reducing the samples by removing the artifacts and soften the resulting soft shadows. Both the CSMs technique and the PCF filtering freed-up significant computing time. The results show that our technique improves efficiency by creating high-quality soft shadows in real-time, and is undoubtedly faster than traditional techniques.

Language used is C++, Opengl Version 2.02, GLSL Version 120. / Visual Studio 2010.

<h2> Ploting the data
  
### The following plots are created by *Imdb_Data_Analysis.py*
  
![2020's movies and shows ratings](Images/2020sMoviesAndShowsRatings.png)  
 
  As we can see here, there are a lot of movies that have a good ratings.
 
![2020's movies and shows metascore](Images/2020sMoviesAndShowsMetascore.png) 
 
  Same thing for metascore.
 
  ![2020's movies and shows ratings](Images/2020sMoviesAndShowsRatingsBar.png) 
  
  To make it much clearer, i added a bar chart.
  
  

