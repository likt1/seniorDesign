% Init Values
sec = 30;
sample_rate = 44100;
bit = 12;

samplesN = sec * sample_rate;
samples = zeros(samplesN,1);

% Generate random 12 bit unsigned values
for i = 1:samplesN
    samples(i) = randi(2^bit);
end

% Convert 12 bit to 16 bit
samples = samples.*16;

% Save as binary file
fileID = fopen('gen-noise.raw','w');
fwrite(fileID, samples, 'uint16');
fclose(fileID);