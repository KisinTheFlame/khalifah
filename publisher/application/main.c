/**************************************************************
 *
 * Example C Prog for Testing EX1629
 * Parameters Set --> Scanlist 48 Ch;
 *                    Samp Freq 1000 Hz;
 * 	                  Input Range 15 V;
 *                    No Bridge Conditioning / Voltage Reading;
 *                    FIFO Data Display
 *
 **************************************************************/

#include <libex1629.h>
#include <stddef.h>
#include <unistd.h>

#define NUM_CHS 48

int ex1629_example() {
    const char *host = "192.168.0.1";
    EX1629_CLIENT *cl = NULL;
    cl = ex1629_clnt_create(host, LID_NONE);

    if (cl != NULL) {
        /* Initial Resetting of EX1629 */
        libex1629_exreset(cl);

        /* Get Serial Number */
        char serial[255];
        libex1629_get_serial_num(cl, serial);
        printf("EX1629 Serial: %s\n", serial);

        /* Set scanlist to desired channels */
        int i, num_chs = 48;
        int new_scanlist[48];
        for (i = 0; i < num_chs; i++)
            new_scanlist[i] = i;
        libex1629_allow_bad_channels(cl);
        libex1629_set_scanlist(cl, num_chs, new_scanlist);

        /* Check if Scanlist has been set */
        int nchans = 0, scanlist[48];
        libex1629_get_scanlist(cl, &nchans, scanlist);
        printf("No. of Enabled Channels: %d\n", nchans);
        printf("The Channels enabled are ");
        for (i = 0; i < nchans; i++)
            printf("%d ", scanlist[i]);
        printf("\n");

        /* Set trigger parameters */
        int32_t pretrig_sample_count = 0;
        uint32_t sample_count = 1;
        uint32_t trig_delay = 0;
        uint32_t arm_delay = 0;
        uint32_t trig_count = 3;
        uint32_t arm_count = 1;
        uint32_t trig_timer_period = 0;
        uint32_t init_cont = 0;
        libex1629_set_trigger_param(
            cl, pretrig_sample_count, sample_count, trig_delay, arm_delay,
            trig_count, arm_count, trig_timer_period, init_cont
        );

        /* Setting up for Voltage Measurement */

        /* Setting Input Mux type to Full Bridge for Voltage Measurement */
        libex1629_set_inputmux(
            cl, num_chs, new_scanlist, INPUTMUX_BRIDGE_TYPE_FULL
        );

        /* Setting Conversion */
        libex1629_set_conversion(cl, num_chs, new_scanlist, EUCONV_VOLT_OUTPUT);

        /* Disabling Excitation */
        libex1629_set_excitation_enbl(cl, num_chs, new_scanlist, 0, 0);

        /* Gain Setting, for input Voltage Range */
        libex1629_set_gain(
            cl, num_chs, new_scanlist,
            1
        ); // Gain = 1 ; Voltage Range = 15V

        /* Setting Sampling Frequency */
        libex1629_set_samp_freq(cl, 1000);

        /* Enabling Soft Sync */
        libex1629_soft_sync(cl);

        /* Inititate the measurement */
        libex1629_triginit(cl);
        sleep(1);

        /* Get the data from the FIFO */
        int fifo_count;
        rpc_fifoarray *fifo_data = malloc(sizeof(rpc_fifoarray));

        int set_page_count = 3; // 1 Page can Store Data of 16 channels
        libex1629_readfifo(cl, set_page_count, &fifo_count, fifo_data);

        /* Unpack the data from the RPC data structure */
        int page_count = fifo_data->datapages.datapages_len, k = 0;
        rpc_datapage page;
        rpc_dataset *data;
        float samples[16];

        printf("Page Count = %d \n", page_count);
        printf("Pages Remaining in FIFO Buffer = %d \n", fifo_count);
        for (i = 0; i < page_count; i++) {
            page = fifo_data->datapages.datapages_val[i];
            data = page.dataset.dataset_val[i];
            int data_count = data->data.data_len;
            int j;
            for (j = 0; j < data_count; j++) {
                samples[j] = data->data.data_val[j];
                printf("Channel: %d \t Data: %f \n", scanlist[k], samples[j]);
                k++;
                if (k > nchans) {
                    k = 0;
                }
            }
        }
        libex1629_free_fifoarray(fifo_data);

        ex1629_clnt_destroy(cl);
    } else {
        perror("Unable to connect.\n");
    }
    return 0;
}

int main() {
    printf("\n...............................EX1629 "
           "Testing.................................. \n");
    ex1629_example();
    return 0;
}
