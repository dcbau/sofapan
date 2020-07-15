/*
 ==============================================================================
 
 SOFAData.cpp
 Created: 4 Apr 2017 11:47:12am
 Author:  David Bau
 
 ==============================================================================
 */

#include "SofaData.h"


int getIRLengthForNewSampleRate(int IR_Length, int original_SR, int new_SR);
//void error(char* errorMessage);
int sampleRateConversion(float* inBuffer, float* outBuffer, int n_inBuffer, int n_outBuffer, int originalSampleRate, int newSampleRate);

SOFAData::SOFAData(){
    
    loadedHRIRs = NULL;
    currentFilePath = NULL;
    currentSampleRate = 0;
    
    interpolatedReturnMagSpectrum = NULL;
    
}

SOFAData::~SOFAData(){
    
    if(loadedHRIRs != NULL){
        for(int i = 0; i < sofaMetadata.numMeasurements; i++){
            delete loadedHRIRs[i];
        }
        free(loadedHRIRs);
    }
    
    if(interpolatedReturnMagSpectrum != NULL)
        free(interpolatedReturnMagSpectrum);
}

int SOFAData::initSofaData(const char* filePath, int sampleRate)
{
    
    printf("\n Attempting to load SofaFile...");
    
    //This line makes sure that the init is not always performed when a new instance of the plugin is created
    if(filePath == currentFilePath && sampleRate == currentSampleRate)
        return 0;
    
    printf("\n Loading File");
    
    
    if(loadedHRIRs != NULL){
        for(int i = 0; i < sofaMetadata.numMeasurements; i++){
            delete loadedHRIRs[i];
        }
        free(loadedHRIRs);
    }

    
    
    
    // LOAD SOFA FILE
    int status = loadSofaFile(filePath, sampleRate);
    if(status){
        errorHandling(status);
        createPassThrough_FIR(sampleRate);
    }
    
    
    
    
    //Normalisation for very high amplitudes, because some sofa files provide integer or too high sample values
    float maxValue = 0.0;
    for(int i = 0; i < sofaMetadata.numMeasurements; i++)
    {
        for(int j = 0; j < lengthOfHRIR * 2; j++)
        {
            if (fabs(loadedHRIRs[i]->getHRIR()[j]) > maxValue)
                maxValue = fabs(loadedHRIRs[i]->getHRIR()[j]);
        }
    }
    
    float normalisation = maxValue > 100 ? 1.0 / maxValue : 1.0;
    
    for(int i = 0; i < sofaMetadata.numMeasurements; i++)
    {
        for(int j = 0; j < lengthOfHRIR * 2; j++)
            loadedHRIRs[i]->getHRIR()[j] *= normalisation;
    }
    
    

    
    
    
    MinPhaseGenerator mpg;    
    
    for(int i = 0; i < sofaMetadata.numMeasurements; i++){
        
        //Create and Copy original HRIR for the pseudo minimum phase HRIRs
        for(int j = 0; j < lengthOfHRIR*2; j++)
            loadedHRIRs[i]->getHRIRZeroITD()[j] = loadedHRIRs[i]->getHRIR()[j];
        
        //Detect Onset Threshold and make Pseudo Minimum Phase HRIRs
        loadedHRIRs[i]->ITD.onsetL_samples = mpg.makePseudoMinPhaseFilter(loadedHRIRs[i]->getHRIRZeroITD()               , lengthOfHRIR, 0.5);
        loadedHRIRs[i]->ITD.onsetR_samples = mpg.makePseudoMinPhaseFilter(loadedHRIRs[i]->getHRIRZeroITD() + lengthOfHRIR, lengthOfHRIR, 0.5);
        
        //Save ITD in ms
        //Negative Values represent a position to the left of the head (delay left is smaller), positive values for right of the head
        loadedHRIRs[i]->ITD.ITD_ms = (loadedHRIRs[i]->ITD.onsetL_samples - loadedHRIRs[i]->ITD.onsetR_samples) * 1000.0 / sampleRate;
        
        
        
        
        //Make the real Minphase HRIRs
        mpg.makeMinPhaseFilter(loadedHRIRs[i]->getHRIR()               , loadedHRIRs[i]->getHRIRMinPhase()               , lengthOfHRIR);
        mpg.makeMinPhaseFilter(loadedHRIRs[i]->getHRIR() + lengthOfHRIR, loadedHRIRs[i]->getHRIRMinPhase() + lengthOfHRIR, lengthOfHRIR);

        
    }
    
    
    //BEGIN THE TRANSFORMATION!
    
    //Allocate and init FFTW
    float* fftInputBuffer = fftwf_alloc_real(lengthOfFFT);
    if(fftInputBuffer == NULL){
        ErrorHandling::reportError("SOFA File Loader", "Could not allocate memory for FFT Input Buffer", true);
        return 1;
    }
    fftwf_complex* fftOutputBuffer = fftwf_alloc_complex(lengthOfHRTF);
    if(fftOutputBuffer == NULL){
        ErrorHandling::reportError("SOFA File Loader", "Could not allocate memory for FFT Output Buffer", true);
        return 1;
    }
    fftwf_plan FFT = fftwf_plan_dft_r2c_1d(lengthOfFFT, fftInputBuffer, fftOutputBuffer, FFTW_ESTIMATE);
    
 
    
    
    for(int i = 0; i < sofaMetadata.numMeasurements; i++){
        
        
        
        // 1 Normal Version
        
        
        // LEFT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIR()[k];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTF()[j][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTF()[j][1] = fftOutputBuffer[j][1]; //IM
        }
        //Save Spectrum for plotting
        for(int k = 0; k < lengthOfHRTF; k++){
            loadedHRIRs[i]->getMagSpectrum()[k] = (sqrtf(fftOutputBuffer[k][0] * fftOutputBuffer[k][0] + fftOutputBuffer[k][1] * fftOutputBuffer[k][1]));
            loadedHRIRs[i]->getPhaseSpectrum()[k] = atan2f(fftOutputBuffer[k][1], fftOutputBuffer[k][0]);// / M_PI;
        }
        PhaseWrapping::unwrap(loadedHRIRs[i]->getPhaseSpectrum(), loadedHRIRs[i]->getPhaseSpectrumUnwrapped(), lengthOfHRTF, false);
        
        
        // RIGHT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIR()[k + lengthOfHRIR];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTF()[j+lengthOfHRTF][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTF()[j+lengthOfHRTF][1] = fftOutputBuffer[j][1]; //IM
        }
        
        //Save Spectrum for plotting
        for(int k = 0; k < lengthOfHRTF; k++){
            loadedHRIRs[i]->getMagSpectrum()[k + lengthOfHRTF] = (sqrtf(fftOutputBuffer[k][0] * fftOutputBuffer[k][0] + fftOutputBuffer[k][1] * fftOutputBuffer[k][1]));
            loadedHRIRs[i]->getPhaseSpectrum()[k + lengthOfHRTF] = atan2f(fftOutputBuffer[k][1], fftOutputBuffer[k][0]);// / M_PI;
        }
        PhaseWrapping::unwrap(loadedHRIRs[i]->getPhaseSpectrum()+lengthOfHRTF, loadedHRIRs[i]->getPhaseSpectrumUnwrapped()+lengthOfHRTF, lengthOfHRTF, false);
        
        
        
        
        
        
        
        // 2 Pseudo Minimum Phase Version
        
        // LEFT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIRZeroITD()[k];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);

        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTFPseudoMinPhase()[j][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTFPseudoMinPhase()[j][1] = fftOutputBuffer[j][1]; //IM
        }

        // RIGHT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIRZeroITD()[k + lengthOfHRIR];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTFPseudoMinPhase()[j+lengthOfHRTF][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTFPseudoMinPhase()[j+lengthOfHRTF][1] = fftOutputBuffer[j][1]; //IM
        }
        
        
        
        
        
        
        // 3 Minimum Phase Version
        
        // LEFT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIRMinPhase()[k];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTFMinPhase()[j][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTFMinPhase()[j][1] = fftOutputBuffer[j][1]; //IM
        }
        
        // RIGHT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getHRIRMinPhase()[k + lengthOfHRIR];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTFMinPhase()[j+lengthOfHRTF][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTFMinPhase()[j+lengthOfHRTF][1] = fftOutputBuffer[j][1]; //IM
        }

        
        for(int k = 0; k < lengthOfHRTF * 2; k++){
            loadedHRIRs[i]->getPhaseSpectrumMinPhase()[k] = atan2f(loadedHRIRs[i]->getHRTFMinPhase()[k][1], loadedHRIRs[i]->getHRTFMinPhase()[k][0]);// / M_PI;
        }

        
    }
    
    fftwf_free(fftInputBuffer);
    fftwf_free(fftOutputBuffer);
    fftwf_destroy_plan(FFT);
 
    
    
    interpolatedReturnMagSpectrum = (float*)malloc(lengthOfHRTF * 2 * sizeof(float));

    
    return 0;
    
}
int SOFAData::getLengthOfHRIR(){
    return lengthOfHRIR;
}




void SOFAData::searchClosestHRTFs(int* results, float* distances, int desiredClosestPoints, float elevation, float azimuth, float radius){
    
    for(int i = 0; i < desiredClosestPoints; i++)
        results[i] = 0;
    
    //transform spherical coordinates to cartesian coordinates
    radius = loadedHRIRs[0]->Distance;
    float x = radius * cosf(elevation * d2r) * cosf(azimuth * d2r);
    float y = radius * cosf(elevation * d2r) * sinf(azimuth * d2r);
    float z = radius * sinf(elevation * d2r);
    
    
    float delta;
    for(int i = 0; i < desiredClosestPoints; i++)
        distances[i] = 1000.0; //start with a random but very high value
    
    // Walk through every measurement and compare its position coordinates with the coordinates we are looking for
    for(int i = 0; i < sofaMetadata.numMeasurements; i++)
    {
        
        
        float dx = x - loadedHRIRs[i]->x;
        float dy = y - loadedHRIRs[i]->y;
        float dz = z - loadedHRIRs[i]->z;
        
        delta = sqrtf(dx*dx + dy*dy + dz*dz);
        
        //check if distance delta is smaller than one of the closest points
        for(int n = 0; n < desiredClosestPoints; n++)
        {
            if( delta < distances[n] )
            {
                //shift other vaues
                for(int m = desiredClosestPoints - 1; m > n; m--)
                {
                    distances[m] = distances[m - 1];
                    results[m] = results[m - 1];
                }
                
                distances[n] = delta;
                results[n] = i;
                
                break;
            }
        }
        
        printf("\n");
        for(int i = 0; i < desiredClosestPoints; i++){
            printf("[%d]: %.1f|%.1f  D: %.3f \n", i, loadedHRIRs[results[i]]->Azimuth, loadedHRIRs[results[i]]->Elevation, distances[i]);
        
        }
        
        
    }
    
}

//same algorithm as above, but only one closest position is returned
int SOFAData::searchHRTF(float elevation, float azimuth, float radius){
    
    int best_id = 0;
    
    
    float x = radius * cosf(elevation * d2r) * cosf(azimuth * d2r);
    float y = radius * cosf(elevation * d2r) * sinf(azimuth * d2r);
    float z = radius * sinf(elevation * d2r);
    
    float delta;
    float min_delta = 1000;
    for(int i = 0; i < sofaMetadata.numMeasurements; i++){
        
        float dx = x - loadedHRIRs[i]->x;
        float dy = y - loadedHRIRs[i]->y;
        float dz = z - loadedHRIRs[i]->z;
        
        delta = dx*dx + dy*dy + dz*dz;
        
        if(delta < min_delta){
            min_delta = delta ;
            best_id = i;
        }
        
    }
    
    return best_id;
    
}





void SOFAData::getHRTFsForInterpolation(float** resultsMag, float** resultsPhase, float* distances, float elevation, float azimuth, float radius, int numDesiredHRTFs, bool minPhase){
    
    int *id = new int[numDesiredHRTFs];
    radius = 1.0;
    searchClosestHRTFs(id, distances, numDesiredHRTFs, elevation, azimuth, radius);
    
    if(minPhase){
        for(int i = 0; i < numDesiredHRTFs; i++){
            resultsMag[i] = loadedHRIRs[id[i]]->getMagSpectrum();
            resultsPhase[i] = loadedHRIRs[id[i]]->getPhaseSpectrumMinPhase();
        }
    }else{
        for(int i = 0; i < numDesiredHRTFs; i++){
            resultsMag[i] = loadedHRIRs[id[i]]->getMagSpectrum();
            resultsPhase[i] = loadedHRIRs[id[i]]->getPhaseSpectrumUnwrapped();
        }
    }

}


fftwf_complex* SOFAData::getHRTFforAngle(float elevation, float azimuth, float radius, int version){
    
    int id = searchHRTF(elevation, azimuth, radius);
    
    switch(version){
        case hrtf_type_original:
            return loadedHRIRs[id]->getHRTF();
        case hrtf_type_zero_itd:
            return loadedHRIRs[id]->getHRTFPseudoMinPhase();
        case hrtf_type_minPhase:
            return loadedHRIRs[id]->getHRTFMinPhase();
        default:
            return NULL;
    }
            
}

float* SOFAData::getHRIRforAngle(float elevation, float azimuth, float radius, int version){
    
    int id = searchHRTF(elevation, azimuth, radius);
    
    switch(version){
        case hrtf_type_original:
            return loadedHRIRs[id]->getHRIR();
        case hrtf_type_zero_itd:
            return loadedHRIRs[id]->getHRIRZeroITD();
        case hrtf_type_minPhase:
            return loadedHRIRs[id]->getHRIRMinPhase();
        default:
            return NULL;
    }
}

ITDStruct SOFAData::getITDForAngle(float elevation, float azimuth, float radius){
    
    int id = searchHRTF(elevation, azimuth, radius);
    return loadedHRIRs[id]->ITD;
}


// This function is not used by the interpolation for the renderer, but for displaying it in the PlotHRTFComponent Class
float* SOFAData::getInterpolatedMagSpectrumForAngle(float elevation, float azimuth, float radius){
    
    
    if(elevation == last_elevation && azimuth == last_azimuth && radius == last_radius)
        return interpolatedReturnMagSpectrum;
    
    last_radius = radius;
    last_azimuth = azimuth;
    last_elevation = elevation;
    
    int id[3];
    float d[3];
    searchClosestHRTFs(id, d, 3, elevation, azimuth, radius);
    
    float* hrtf1 = loadedHRIRs[id[0]]->getMagSpectrum();
    float* hrtf2 = loadedHRIRs[id[1]]->getMagSpectrum();
    float* hrtf3 = loadedHRIRs[id[2]]->getMagSpectrum();

    float w1, w2, w3;
    
    //if one distance is zero, no interpolation is done at all
    if(d[0] == 0 ||d[1] == 0 ||d[2] == 0 ){
        w1 = d[0] == 0 ? 1 : 0;
        w2 = d[1] == 0 ? 1 : 0;
        w3 = d[2] == 0 ? 1 : 0;
    }else{
        w1 = 1.0 / d[0];
        w2 = 1.0 / d[1];
        w3 = 1.0 / d[2];
    }
    
    for(int i = 0; i < lengthOfHRTF * 2; i++){
        interpolatedReturnMagSpectrum[i] = (hrtf1[i] * w1 + hrtf2[i] * w2 + hrtf3[i] * w3) / (w1 + w2 + w3);
        //interpolatedReturnMagSpectrum[i + lengthOfHRTF] = loadedHRIRs[id1]->getMagSpectrum()[i];
    }
    
    return interpolatedReturnMagSpectrum;

}

float* SOFAData::getPhaseSpectrumForAngle(float elevation, float azimuth, float radius, int version){
    
    
    int id = searchHRTF(elevation, azimuth, radius);
    switch (version) {
        case hrtf_type_original:
            return loadedHRIRs[id]->getPhaseSpectrum();
        case hrtf_type_original_unwrapped:
            return loadedHRIRs[id]->getPhaseSpectrumUnwrapped();
        case hrtf_type_minPhase:
            return loadedHRIRs[id]->getPhaseSpectrumMinPhase();
        default:
            return NULL;
            
    }
}



int SOFAData::loadSofaFile(const char* filePath, int hostSampleRate){
    
    /* Some things are defined as variables in the SOFA-Convention, but are assumed here for the sake of simplicity:
     - Data.SamplingRate:Units is always "hertz"
     - Data.Delay is Zero, the TOA information is contained in the FIRs
     - Data.IR has always three dimensions
     - Data.IR has two receivers = two ears
     
     Furthermore, by now the only accepted DataType is "FIR" and SOFAConvention is "SimpleFreeFieldHRIR"
     */
    
    
    
    //Step 1: Open File in read mode (NC_NOWRITE) and get netcdf-ID of Sofa-File
    int  status;               /* error status */
    int  ncid;                 /* netCDF ID */
    if ((status = nc_open(filePath, NC_NOWRITE, &ncid)))
        return ERR_OPENFILE;
    
    
    
    
    //Step 2: Get GLOBAL attributes
    int numberOfAttributes;
    nc_inq(ncid, NULL, NULL, &numberOfAttributes, NULL);
    
	char** name_of_att =  new char*[numberOfAttributes];
    for (int i = 0; i <= numberOfAttributes; ++i) {
        name_of_att[i] = new char[NC_MAX_NAME+1];
    }
    char** attributes = new char*[numberOfAttributes];
    
    sofaMetadata.globalAttributeNames.resize(0);
    sofaMetadata.globalAttributeValues.resize(0);
    
    for(int i = 0; i < numberOfAttributes; i++){
        nc_inq_attname(ncid, NC_GLOBAL, i, name_of_att[i]);
        
        size_t attlength;
        nc_inq_attlen(ncid, NC_GLOBAL, name_of_att[i], &attlength);
        
        char* att = new char[attlength + 1];
        nc_get_att(ncid, NC_GLOBAL, name_of_att[i], att);
        att[attlength] = '\0';
        attributes[i] = att;

        //Check for ö (especially for the FH Köln Dataset ;) )
        for(int k = 0; k < attlength; k++){
            if(att[k] == '\366') //This value is in the attributes where an "ö" should be. It messes up the whole string procedure under windows.
                att[k] = 'o';
        }
        
        
        sofaMetadata.globalAttributeNames.add(String(CharPointer_UTF8 (name_of_att[i])));
        sofaMetadata.globalAttributeValues.add(String(CharPointer_UTF8 (attributes[i])));

    }
    
    sofaMetadata.listenerShortName = getSOFAGlobalAttribute("ListenerShortName", ncid);

    
    /* -- check if attribute "SOFAConventions" is "SimpleFreeFieldHRIR" -- abort if not*/
    String sofa_conv = getSOFAGlobalAttribute("SOFAConventions", ncid);
    if(sofa_conv.compare("SimpleFreeFieldHRIR")){
        nc_close(ncid);
        return ERR_NOTSUP;
    }
    sofaMetadata.SOFAConventions = sofa_conv;
    
    /* -- check if attribute "DataType" is "FIR" -- abort if not */
    String data_type = getSOFAGlobalAttribute("DataType", ncid);
    if(data_type.compare("FIR")){
        nc_close(ncid);
        return ERR_NOTSUP;
    }
    sofaMetadata.dataType = data_type;
    
    //Get Sampling Rate
    int SamplingRate, SamplingRate_id;
    status = nc_inq_varid(ncid, "Data.SamplingRate", &SamplingRate_id);
    status += nc_get_var_int(ncid, SamplingRate_id, &SamplingRate);
    if(status != NC_NOERR){
        //error((char*)"Load Sofa: Could not read Sample Rate");
        nc_close(ncid);
        return ERR_READFILE;
    }
    sofaMetadata.sampleRate = SamplingRate;
    
    
    
    
    //Step 3: Get needed Dimensions
    
    //Get netcdf-ID of IR Data
    int DataIR_id;
    if ((status = nc_inq_varid(ncid, "Data.IR", &DataIR_id)))//Get Impulse Resopnse Data ID
        return ERR_READFILE;
    
    //Get Dimensions of Data IR
    int DataIR_dimidsp[MAX_VAR_DIMS];

    size_t dimM_len;//Number of Measurements
    size_t dimR_len;//Number of Receivers, in case of two ears dimR_len=2
    size_t dimN_len;//Number of DataSamples describing each Measurement
    
    if ((status = nc_inq_var(ncid, DataIR_id, 0, 0, 0, DataIR_dimidsp, 0)))
        return ERR_READFILE;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[0], &dimM_len)))
        return ERR_READFILE;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[1], &dimR_len)))
        return ERR_READFILE;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[2], &dimN_len)))
        return ERR_READFILE;
    
    sofaMetadata.numSamples = dimN_len; //Store Value in Struct metadata_struct
    sofaMetadata.numMeasurements = dimM_len;
    sofaMetadata.numReceivers = dimR_len;
    
    // Übergangslösung: Es werden dateien mit mehereren Receiverkanälen akzepiert, aber statisch die ersten beiden kanäle verwendet
    int receiver_for_L = 0;
    int receiver_for_R = 1;

    
    
    //Step 4:
    //now that the sampleRate of the SOFA, the host sample rate and the length N is known, the length of the interpolated HRTF can be calculated
    lengthOfHRIR = getIRLengthForNewSampleRate(dimN_len, sofaMetadata.sampleRate, hostSampleRate);
    lengthOfFFT = 2 * lengthOfHRIR;
    lengthOfHRTF = (lengthOfFFT * 0.5) + 1;
    
    
    
    //Step 5: Get Source Positions (Azimuth, Elevation, Distance)
    int SourcePosition_id;
    if ((status = nc_inq_varid(ncid, "SourcePosition", &SourcePosition_id)))
        return ERR_READFILE;
    float* SourcePosition = NULL;
    SourcePosition = (float*)malloc(sizeof(float) * 3 * dimM_len); //Allocate Memory for Sourcepositions of each Measurement
    if(SourcePosition == NULL)
        return ERR_MEM_ALLOC;
    if ((status = nc_get_var_float(ncid, SourcePosition_id, SourcePosition)))// Store Sourceposition Data to Array
    {
        free(SourcePosition);
        return ERR_READFILE;
    };
    
    
    
    //Step 6: Get Ear Positions
    int ReceiverPosition_id;
    if ((status = nc_inq_varid(ncid, "ReceiverPosition", &ReceiverPosition_id)))
        return ERR_READFILE;
    float* ReceiverPosition =  NULL;
    ReceiverPosition = (float*)malloc(sizeof(float) * 3 * dimR_len); //Allocate Memory for Sourcepositions of each Measurement
    if(ReceiverPosition == NULL)
        return ERR_MEM_ALLOC;
    if ((status = nc_get_var_float(ncid, ReceiverPosition_id, ReceiverPosition)))// Store Sourceposition Data to Array
    {
        free(SourcePosition);
        free(ReceiverPosition);
        return ERR_READFILE;
    };
    
    float radius1 = ReceiverPosition[1]; // y-koordinate des ersten Receivers (L)
    float radius2 = ReceiverPosition[4]; // y-koordinate des zweiten Receivers (R)
    sofaMetadata.headRadius = (fabsf(radius1) + fabsf(radius2)) / 2.0;
    
    
    
    
    
    //Step 7: Get Impulse Responses
    float *DataIR = NULL;
    DataIR = (float*)malloc(dimM_len * dimR_len * dimN_len * sizeof(float));
    if(DataIR == NULL)
        return ERR_MEM_ALLOC;
    if ((status = nc_get_var_float(ncid, DataIR_id, DataIR))) //Read and write Data IR to variable Data IR
    {
        free(SourcePosition);
        free(ReceiverPosition);
        free(DataIR);
        return ERR_READFILE;
    };
    
    int numZeroElevation = 0;
    int u = 0;
    for (int i = 0; i < dimM_len; i++) {
        if (SourcePosition[u + 1] == 0) {
            numZeroElevation++;
        }
        u += 3;
    }
    sofaMetadata.numMeasurements0ev = numZeroElevation;
    
    loadedHRIRs = (Single_HRIR_Measurement**)malloc(sofaMetadata.numMeasurements * sizeof(Single_HRIR_Measurement));
    if(loadedHRIRs == NULL){
        free(SourcePosition);
        free(DataIR);
        return ERR_MEM_ALLOC;
    }
    
    
    sofaMetadata.minElevation = 0.0;
    sofaMetadata.maxElevation = 0.0;
    sofaMetadata.minDistance = 1000.0;
    sofaMetadata.maxDistance = 0.0;
    
    std::vector<float>differentDistances;
    differentDistances.resize(0);
    
    int i = 0, j = 0, l = 0, x = 0;
    float *IR_Left = (float *)malloc(dimN_len * sizeof(float));
    float *IR_Right = (float *)malloc(dimN_len * sizeof(float));
    float *IR_Left_Interpolated = (float *)malloc(lengthOfHRIR * sizeof(float));
    float *IR_Right_Interpolated = (float *)malloc(lengthOfHRIR * sizeof(float));
    
    for (i = 0; i < dimM_len; i++) {

        if(SourcePosition[l+1] < sofaMetadata.minElevation) sofaMetadata.minElevation = SourcePosition[l+1];
        if(SourcePosition[l+1] > sofaMetadata.maxElevation) sofaMetadata.maxElevation = SourcePosition[l+1];
        if(SourcePosition[l+2] < sofaMetadata.minDistance) sofaMetadata.minDistance = SourcePosition[l+2];
        if(SourcePosition[l+2] > sofaMetadata.maxDistance) sofaMetadata.maxDistance = SourcePosition[l+2];
        
//
        bool distanceAlreadyExists = false;
        for(int n = 0; n < (int)differentDistances.size(); n++)
        {
            if(SourcePosition[l+2] == differentDistances[n])
            {
                distanceAlreadyExists = true;
            }
        }
        if(!distanceAlreadyExists)
        {
            float newDistance = SourcePosition[l+2];
            differentDistances.push_back(newDistance);
        }
        
        
        Single_HRIR_Measurement *measurement_object = new Single_HRIR_Measurement(lengthOfHRIR, lengthOfHRTF);
        //Temporary storage of HRIR-Data
        
        
        for (j = 0; j < dimN_len; j++) {
            IR_Right[j] = DataIR[i*(dimR_len*dimN_len) + receiver_for_R * dimN_len + j];
            
            IR_Left[j] = DataIR[i*(dimR_len*dimN_len) + receiver_for_L * dimN_len + j];
            
        };
        
        sampleRateConversion(IR_Right, IR_Right_Interpolated, dimN_len, lengthOfHRIR, sofaMetadata.sampleRate, hostSampleRate);
        sampleRateConversion(IR_Left, IR_Left_Interpolated, dimN_len, lengthOfHRIR, sofaMetadata.sampleRate, hostSampleRate);
        
        for(int i = 0; i < lengthOfHRIR; i++){
            measurement_object->getHRIR()[i] = IR_Left_Interpolated[i];
            measurement_object->getHRIR()[i+lengthOfHRIR] = IR_Right_Interpolated[i];
        }
        
        
        
        float wrappedSourceposition = fmodf((SourcePosition[l] + 360.0), 360.0);
        
        measurement_object->setValues(wrappedSourceposition, SourcePosition[l + 1], SourcePosition[l + 2]);
        measurement_object->index = i;
        loadedHRIRs[i] = measurement_object;
        
        x++;
        
        l += 3;
    };
    
    sofaMetadata.numDifferentDistances = differentDistances.size();

    sofaMetadata.hasMultipleDistances = sofaMetadata.minDistance != sofaMetadata.maxDistance  ? true : false;
    sofaMetadata.hasElevation = sofaMetadata.maxElevation != sofaMetadata.minElevation ? true : false;

    free(IR_Left);
    free(IR_Right);
    free(IR_Left_Interpolated);
    free(IR_Right_Interpolated);
    
    
    free(ReceiverPosition);
    free(SourcePosition);
    free(DataIR);
    nc_close(ncid);
    
    //SOFAFile_loaded_flag = 1;
    return 0;
    
}



sofaMetadataStruct SOFAData::getMetadata(){
    return sofaMetadata;
}



#pragma mark HELPERS

int sampleRateConversion(float* inBuffer, float* outBuffer, int n_inBuffer, int n_outBuffer, int originalSampleRate, int newSampleRate){
    
//    for(int i = 0; i < n_outBuffer; i++){
//        outBuffer[i] = inBuffer[i];
//    }
//
//    return 0;
    
    float frequenzFaktor = (float)originalSampleRate / (float)newSampleRate;
    float f_index = 0.0f;
    int i_index = 0;
    float f_bruch = 0;
    int i_fganzezahl = 0;
    while ((i_fganzezahl+1) < n_inBuffer){
        if(i_index >= n_outBuffer || (i_fganzezahl+1) >= n_inBuffer) return 1;
        //Linear interpolieren
        outBuffer[i_index] = inBuffer[i_fganzezahl] * (1.0f - f_bruch) + inBuffer[i_fganzezahl + 1] * f_bruch;
        outBuffer[i_index] *= frequenzFaktor;
        //Berechnungen fuer naechste Runde.
        i_index++;
        f_index = i_index * frequenzFaktor;
        i_fganzezahl = (int)f_index;
        f_bruch = f_index - i_fganzezahl;
    }
    while(i_index < n_outBuffer){
        outBuffer[i_index] = 0.0;
        i_index++;
    }
    
    return 0;
    
}


int getIRLengthForNewSampleRate(int IR_Length, int original_SR, int new_SR){
    
    float frequenzFaktor = (float)original_SR / (float)new_SR;
    float newFirLength_f = (float)(IR_Length) / frequenzFaktor; //z.B. 128 bei 44.1kHz ---> 128/(44.1/96) = 278.6 bei 96kHz
    int fir_length = 2;
    while (fir_length < newFirLength_f) {
        fir_length *= 2;
    }
    
    return fir_length;
}


void SOFAData::errorHandling(int status) {
    String ErrorMessage;
    switch (status) {
        case ERR_MEM_ALLOC:
            ErrorMessage = "Error: Memory allocation";
            break;
        case ERR_READFILE:
            ErrorMessage = "Error while reading from File";
            break;
        case ERR_NOTSUP:
            ErrorMessage = "This file contains data that is not supported";
            break;
        case ERR_OPENFILE:
            ErrorMessage = "Could not open file";
            break;
        case ERR_UNKNOWN:
            ErrorMessage = "An Error occured during loading the File";
            break;
        default:
            ErrorMessage = "An Error occured during loading the File";
            break;
    }
    
    ErrorMessage += String(". Try reloading the file");
    
    
    ErrorHandling::reportError("SOFA File Loader", ErrorMessage, false);
    
    
    
}


//This is needed if no sofa file can be loaded, as a backup solution. It mimics a loaded sofa file with only one measurement - a pass through fir filter
void SOFAData::createPassThrough_FIR(int _sampleRate){
    
    const int passThroughLength = 512;
    sofaMetadata.sampleRate = _sampleRate;
    sofaMetadata.numMeasurements = 1;
    sofaMetadata.numSamples = passThroughLength;
    sofaMetadata.dataType = String ("FIR");
    sofaMetadata.SOFAConventions = String ("Nope");
    sofaMetadata.listenerShortName = String ("Nope");
    
    sofaMetadata.minElevation =0.0;
    sofaMetadata.maxElevation =0.0;
	sofaMetadata.minDistance = 1.0;
	sofaMetadata.maxDistance = 1.0;
    
    lengthOfHRIR = passThroughLength;
    lengthOfFFT = 2 * lengthOfHRIR;
    lengthOfHRTF = (lengthOfFFT * 0.5) + 1;
    
    loadedHRIRs = (Single_HRIR_Measurement**)malloc(1 * sizeof(Single_HRIR_Measurement));
    Single_HRIR_Measurement *measurement_object = new Single_HRIR_Measurement(lengthOfHRIR, lengthOfHRTF);
    
    float *IR_Left = (float *)malloc(passThroughLength * sizeof(float));
    float *IR_Right = (float *)malloc(passThroughLength * sizeof(float));
    
    IR_Left[0]  = 1.0;
    IR_Right[0] = 1.0;
    for (int j = 1; j < passThroughLength; j++) {
        IR_Left[j]  = 0.0;
        IR_Right[j] = 0.0;
    }
    
    
    for(int i = 0; i < lengthOfHRIR; i++){
        measurement_object->getHRIR()[i] = IR_Left[i];
        measurement_object->getHRIR()[i+lengthOfHRIR] = IR_Right[i];
    }
    
    measurement_object->setValues(0.0, 0.0 , 0.0);
    measurement_object->index = 0;
    loadedHRIRs[0] = measurement_object;
    free(IR_Left);
    free(IR_Right);
    
}

String SOFAData::getSOFAGlobalAttribute(const char* attribute_ID, int ncid){
    size_t att_length = 0;
    
    //get length if possible
    
    
    if(nc_inq_attlen(ncid, NC_GLOBAL, attribute_ID, &att_length))
        return String("- Unknown - ");
    
    if(att_length == 0)
        return String("Empty");
    
    //get value if possible
    char* att = new char[att_length + 1];
    if(nc_get_att(ncid, NC_GLOBAL, attribute_ID, att))
        return String("- Unknown - ");
    
    
    
    //terminate string manually
    att[att_length] = '\0';
    
    return String(att);
}

