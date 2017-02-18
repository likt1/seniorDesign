% Init Values
sec = 10;
sample_rate = 44100;
time = 0:1/sample_rate:sec;
freq = 440;
bit = 8;

% Generate sin 440Hz wave
samples = sin(2.*pi.*freq.*time);
samples = samples.*(2^bit/2) + (2^bit/2);

% Convert 12 bit to 16 bit
%samples = samples.*16;

% Save as binary file
fileID = fopen('gen-sin-8bit.raw','w');
fwrite(fileID, samples, 'uint8');
fclose(fileID);

% plot
plot(time, samples);