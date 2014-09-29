PS2-VU-Lighting
===============
Directional lighting using the Vector Unit of the PS2

The application makes use of the PS2 quad buffering scheme, where the VU (Vector Unit) memory is split into two main buffers and these two buffers are then divided into two smaller buffers. This scheme efficiently uses the PS2’s parallel architecture to give the best throughput, and this means that the uploading of the vertex data from the DMAC (Direct Memory Access Controller) to the VU where it isprocessed and then to the GS (Graphics Synthesiser) to be rendered.
