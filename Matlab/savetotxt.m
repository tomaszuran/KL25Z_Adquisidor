mod = 48000;
ps = 0;
n = 100;

freq_muestreo = 48000000 / (mod * 2^ps)

config = ['$M'  num2str(mod, '%05d') 'P' num2str(ps) 'N' num2str(n, '%05d') '#'];

s = serial('COM3','BaudRate',57600);

fclose(s);
fopen(s);
fprintf(s, config);

data = zeros(1, n);

for i = 1:n
    data(i) = str2double(fscanf(s));
    
end

fclose(s);

%fileID = fopen('out.txt','w');
%fprintf(fileID,i);
%fclose(fileID);
