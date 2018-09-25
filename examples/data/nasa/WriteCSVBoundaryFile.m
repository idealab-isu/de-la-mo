function WriteCSVBoundaryFile(filename, A)

fid = fopen(filename, 'w');
fprintf(fid, '"Points:0","Points:1","Points:2"\n');
fclose(fid);
dlmwrite(filename, A, '-append', 'precision', '%.6f', 'delimiter', ',');