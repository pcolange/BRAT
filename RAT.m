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
    
  distortion_knob = 0.5;
  filter_knob = 1.0;
  volume_knob = 1.0; % note used for now... 
   
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
 
 filename = "C:/Users/phili/Desktop/Filter/CEGtr.wav"; 
 %filename = "C:/Users/phili/Desktop/Filter/AC30 Live 2.wav";
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
 
  % Normalize the input to +- 1
  max_y = max(abs(input));
  scale = 1 / max_y;
  input *= scale;  

    % OP AMP TF
  Cf = 100E-12;
  Rf = 100E3 * distortion_knob;
  C1 = 2.2E-6;
  R1 = 47;
  C2 = 4.7E-6;
  R2 = 560;
   
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
  gbw/gain 

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
  opamp_out = filter(bz, az, input); 
  
  Vcc = 4.95;
  Vswing = 1;
  Cc = 30E-12;
  
  % Using LTSpice I measured the slew_rate to be ~235k
  slew_rate = 235E3; % V/sec SR = I / Cc
    
  dc_gain = 2E5;
  gbw = 1E9; % the freq at which the gain is 1
  pole_freq = gbw / dc_gain; % 5Hz where the gain starts attenuating
  R1 = 1 / (2 * pi * pole_freq * Cc); % http://www.ecircuitcenter.com/OpModels/Ilimit_Slew/ILim_Slew.htm
  K1 = dc_gain / R1; % transconductance gain 
  
  % Once we hit the slew_freq, the input voltage no longer affects the output
  % if Vp is greater than this frequency, then slewing occurs 
  % slew rate doesn't seem affected by VDC?
  slew_freq = slew_rate / (2 * pi * max(max(input)*gain, Vcc)) % when slewing occurs
  %slew_peak = slew_rate / (2 * pi * max(freq)) % largest undistorted signal amp.
  
  %triangle_peak = slew_rate / (max(freq) * 4) % triangle wave peak voltage
  
  psample = opamp_out(1);
  
  slope = zeros(1,length(opamp_out));
  slewdt = slew_freq / Fs; % slew rate cycles/sample
  dt = 1/Fs;
  vmax = slew_rate * dt
  for i=1:length(opamp_out)
    dv = opamp_out(i) - psample;    
    if(dv > vmax)
      dv = vmax;
    endif
    if(dv < -vmax)
      dv = -vmax;
    endif
    psample += dv;    
    opamp_out(i) = psample;
  endfor
  
  % Drive circuit (diodes)
  Is = 1E-14;
  n = 1;
  vt = 26.E-3;
  nvt = n * vt;
  R = 1E3;
  U = nvt * asinh(opamp_out/(2*Is*R));

  for i=1:length(U)
    if(abs(U(i)) > abs(opamp_out(i)))
      U(i) = opamp_out(i);
    endif    
  endfor
  
  % just scale the output... % KNOB 3 Should be the drive 
  max_y = max(abs(U));
  scale = 1 / max_y;
  U *= scale;  
  
  % TONE SECTION - LPF
  Rtone = 100E3 * filter_knob + 1.5E3;
  Ctone = 3.3E-9;
  num = [1];
  den = [Rtone * Ctone, 1];
  tonef = tf(num, den); 
  
  [bz, az] = bilinear(num, den, 1/Fs);
  U = filter(bz, az, U); 
 
  player = audioplayer(U, Fs);
  play(player);
  
  figure(1)
  subplot(2, 1, 1)
  plot(t, input, "color", "blue")
  hold on;
  subplot(2, 1, 2)
  plot(t, U, "color", "red")
  
##  title('Input');
##  xlabel('Time (samples)');
##  ylabel('Amplitude (dB)');
##  set(gca, "fontsize", 24);
##  grid on;
  
 