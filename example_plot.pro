PRO example_plot

print,'Press any key to scroll through output plots'
;wdelete,0
window,0,xsize=600,ysize=600,retain=2

; hardwired path and filename for input file
infile='outputs/enf_test1.dayout'

; make sure that file unit 1 is closed
close,1

; open the infile for read
openr,1,infile

years=1000L

; define the type and dimensionality of the binary file, using
; an associated variable
a=assoc(1,fltarr(23,365,years))

; read binary data from infile into a named variable (d)
d=a(0)

; close the input file
close,1

; hardwired variable names for plot titles
names=strarr(23)
names[0] = 'Soil Water (kg H2O/m2)'
names[1] = 'Snow Water (kg H2O/m2)'
names[2] = 'Canopy Water Evaporation (kg H2O/m2/day)'
names[3] = 'Snow Water Sublimation (kg H2O/m2/day)'
names[4] = 'Soil Water Evaporation (kg H2O/m2/day)'
names[5] = 'Transpiration (kg H2O/m2/day)'
names[6] = 'Outflow (kg H2O/m2/day)'
names[7] = 'Coarse Woody Litter (kg C/m2)'
names[8] = 'Projected LAI'
names[9] = 'Net N mineralization (kg N/m2/day)'
names[10] = 'NPP (kg C/m2/day)'
names[11] = 'NEP (kg C/m2/day)'
names[12] = 'NEE (kg C/m2/day)'
names[13] = 'GPP (kg C/m2/day)'
names[14] = 'Maintenance Respiration (kg C/m2/day)'
names[15] = 'Growth Respiration (kg C/m2/day)'
names[16] = 'Heterotrophic Respiration (kg C/m2/day)'
names[17] = 'Fire Losses (kg C/m2/day)'
names[18] = 'Vegetation Carbon (kg C/m2)'
names[19] = 'Litter Carbon (kg C/m2)'
names[20] = 'Soil Carbon (kg C/m2)'
names[21] = 'Total Carbon (kg C/m2)'
names[22] = 'sunlit leaf-level Assim. (umol CO2/m2/sec)'

; define some time-axis information
time1 = (float(indgen(365*years))/365.0) + 1950.0
time2 = indgen(365)
time3 = indgen(years)+1950

; loop to plot each of the 23 variables
for i=0,22 do begin

	; first plot the daily data for all 44 years
	;plot,time1,d(i,*,*),/yno,title=names[i],xr=[1950,1950+years],/xstyle
	; wait for a key press
	;junk=get_kbrd(1)

        ; first plot the daily data for all 44 years
	plot,d(i,*,*),/yno,title=names[i],xr=[1950,1950+years],/xstyle
	; wait for a key press
	junk=get_kbrd(1)

	; next make an average year and plot that
	avgyr = reform(total(d(i,*,*),3))/float(years)
	plot,time2,avgyr,/yno,title=names[i]+' : Average Year',xr=[0,364],xs=1,$
		xtitle='yearday'
	junk=get_kbrd(1)

	; next make an annual average timeseries
	annavg = reform(total(d(i,*,*),2))/365.0
	plot,time3,annavg,/yno,title=names[i]+' : Yearly Average',xr=[1950,1950+years],$
		xs=1
	junk=get_kbrd(1)
endfor
return
end
