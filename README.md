# RayTracing
CPU-based RayTracing app (yea another one)
![изображение](https://github.com/LaLKa1/RayTracing/assets/63583139/b1806e7e-89cb-43c9-b029-0d96635b8585)

Currently done:
  Raw multithreading calculation
  finding intersection coordinates in one step using a lot of vector math
  some basic optimization (like several simple checks of intersection before actually calculate it)
In the work:
  auto BVH generating
TODO:
  actually lightning system (at this moment color based on distance to camera)
  postprocessing
  more primitive types (not only sphere and plane)
After all of this:
  Moving to GPU calculation (CUDA/OpenGL/OpenCL/Vulcan)
  In-app scene redactor (wtf rtx-based game-engine??)
