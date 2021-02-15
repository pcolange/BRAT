####1  % Modeling the RAT guitar distortion pedal 
 
 % Slew rate modeling
 %% Slew rate is a fixed variable measured in V/s or V/us and can be 
 %% taken from the data sheet for the LM308N OpAmp.
 %% SR = 2*pi*f*V where f and V are highest frequency and Voltage before slewing
 %% this equation can be used to find max freq based on input voltage etc...
 %% The input DC offset does affect 
 
 %% I think I have a good handle on slew rate at this point. The equations
 %% that return slew_freq and triangle_peak seem to be ok. 
 %% Slew is affected by the closed loop gain but will be clipped at Vcc
 %% Pretty sure DC Bias will be removed in derivation but should run some 
 %% experiments to see 0->9 4.5 bias results, I think it will act like 
 %% -4.5->4.5 but not sure. 
 %% Samples provide timestep and voltage level to determine slew effect
 %% But slew is not applied until a freq threshold... 
 
 
 clc
 clear
 close all
 
 % Helper Function in place of bilinear() 
 function [bz, az] = mybi(b2,b1,b0,a2,a1,a0, Fs)
  c= 2*Fs;
  c_sq = power(c,2);
  
  az0 = (a2*c_sq + a1*c + a0);
   
  bz0 = (b2*c_sq + b1*c + b0) / az0;
  bz1 = (-2*b2*c_sq + 2*b0) / az0;
  bz2 = (b2*c_sq - b1*c + b0) / az0;

  az1 = (-2*a2*c_sq + 2*a0)/az0;
  az2 = (a2*c_sq - a1*c + a0)/az0;
  az0 = 1   ;
   
  bz = [bz0, bz1, bz2];
  az = [az0, az1, az2];
 endfunction
 
 %filename = "CEGtr.wav"; 
 filename = "C:/Users/phili/Desktop/Filter/AC30 Live 2.wav";
 %filename = "C:/Users/phili/Desktop/Melodies of the Dead-Master.wav";
 wav_info = audioinfo(filename);
 seconds = 5;
 samples_vec = [1, seconds * wav_info.SampleRate]; % for 10 seconds
 samples = samples_vec(2);
 
 [input, Fs] = audioread(filename, samples_vec);
 
 if wav_info.NumChannels == 2
   input = input(:,1);
 endif
 
  t = 0:1/Fs:seconds-1/Fs; 
 
 %Fs = 44100*2; % sampling rate
 %A = 1; % Volts
 %freq = [1E3]; % Frequencies for in signals
 %cycles = 15;
 %samples = cycles * Fs/freq;
 %t = 0 : 1/Fs : (samples/Fs - (1/Fs)); % length of input signal
 %input = zeros(1,length(t)); % initialize

 %for i = freq
 % input += A * sin(2*pi*i*t);
 %endfor
 
  % Normalize the input to +- 1
  max_y = max(abs(input));
  scale = 1 / max_y;
  input *= scale;
 
  distortion_knob = 1.0;
  tone_knob = 1.0;
  volume_knob = 1.0;
   
  opamp_in = transpose(input); %filter(bz, az, input);
   
  % OP AMP TF
  Cf = 100E-12;
  Rf = 100E3;
  C1 = 2.2E-6;
  R1 = 47;
  C2 = 4.7E-6;
  R2 = 560;

  % From Sapwin
##+ (  +1) 
##+ (  C1 Rf + C1 R1 + C2 Rf + C2 R2 + Cf Rf ) s
##+ (  C2 C1 R2 Rf + C2 C1 R1 Rf + C2 C1 R1 R2 + Cf C1 R1 Rf + Cf C2 R2 Rf ) s^2
##+ (  Cf C2 C1 R1 R2 Rf ) s^3
##------------------------------------------------------------------------------
##+ (  +1 )
##+ (  C1 R1 + C2 R2 + Cf Rf ) s
##+ (  C2 C1 R1 R2 + Cf C1 R1 Rf + Cf C2 R2 Rf ) s^2
##+ (  Cf C2 C1 R1 R2 Rf ) s^3

##+ (  +1 )
##+ (  C1 Rf + C1 R1 + C2 Rf + C2 R2 + Cf Rf ) s
##+ (  C2 C1 R2 Rf + C2 C1 R1 Rf + C2 C1 R1 R2 + Cf C1 R1 Rf + Cf C2 R2 Rf ) s^2
##+ (  Cf C2 C1 R1 R2 Rf ) s^3
##------------------------------------------------------------------------------
##+ (  +1 )
##+ (  C1 R1 + C2 R2 + Cf Rf + Chp Rhp ) s
##+ (  C2 C1 R1 R2 + Cf C1 R1 Rf + Chp C1 R1 Rhp + Cf C2 R2 Rf + Chp C2 R2 Rhp + Chp Cf Rf Rhp ) s^2
##+ (  Cf C2 C1 R1 R2 Rf + Chp C2 C1 R1 R2 Rhp + Chp Cf C1 R1 Rf Rhp + Chp Cf C2 R2 Rf Rhp ) s^3
##+ (  Chp Cf C2 C1 R1 R2 Rf Rhp ) s^4

  % The closed loop gain
  Rp = R1 * R2 / (R1 + R2);
  gain = 1 + Rf / Rp
  gain_db = 20 * log10(gain)
       
  % Model the OpAmps GBW
  open_gain = 2E5; % from datasheet
  gbw = 1E6; % Hz, fx, approx from datasheet
  wx = 2*pi*gbw; % radian freq
  w0 = wx / gain; 
  f0 = w0 / (2*pi)

  RCgbw = 1/(2*pi*f0);

  num1 = [Cf*C2*C1*R1*R2*Rf,         
         C2*C1*R2*Rf + C2*C1*R1*Rf + C2*C1*R1*R2 + Cf*C1*R1*Rf + Cf*C2*R2*Rf, 
         C1*Rf + C1*R1 + C2*Rf + C2*R2 + Cf*Rf, 
         1];
  den1 = [Cf*C2*C1*R1*R2*Rf*RCgbw,
         Cf*C2*C1*R1*R2*Rf + C2*C1*R1*R2*RCgbw + Cf*C1*R1*Rf*RCgbw + Cf*C2*R2*Rf*RCgbw, 
         C2*C1*R1*R2 + Cf*C1*R1*Rf + C1*R1*RCgbw + Cf*C2*R2*Rf + C2*R2*RCgbw + Cf*Rf*RCgbw,
         C1*R1 + C2*R2 + Cf*Rf + RCgbw,
         1];
   
  [bz, az] = bilinear(num1, den1, 1/Fs);
  opamp_out = filter(bz, az, opamp_in); 
        
  % Simple LPF
  lpf = tf([1], [1E3 * 1E-9, 1]); 
  
  % Construct transfer functions
  tf1 = tf(num1,den1);

  f = logspace(0, 4.3, 200);
  [mag1, pha1] = bode(tf1, 2*pi*f);

  max_y = max(abs(opamp_out));
  scale = 1 / max_y;
  opamp_out *= scale;
    
  player = audioplayer(opamp_out, Fs);
  play(player);
  
  % Input Output
##  figure(1)  
##  subplot(2, 1, 1)  
##  plot(t, opamp_in, "color", "blue")  
##  title('Input')
##  
##  subplot(2, 1, 2)  
##  plot(t, opamp_out, "color", "red")
##  title('Output')
  
  % Filters 
  figure(2)  
  semilogx(f, 20*log10(mag1), "color", "green") % RC Filters
  title('OpAmp Filter Response')
  xlabel("Frequency (Hz)")
  ylabel("Magnitude (dB)")
  grid on
  
##  hold on;
##  semilogx(f, 20*log10(mag2), "color", "black") % GBW limit
##
##  hold on;
##  semilogx(f, 20*log10(mag3), "color", "red") % Combined
  
##  figure(3)
##  semilogx(f, 20*log10(mag2), "color", "black") 
##  title('Simple LPF')
##  xlabel("Frequency (Hz)")
##  ylabel("Magnitude (dB)")
##  grid on;
  
  
 