#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <libcula/core.h>
#include <libcula/services/upower.h>

int main(void) {
    printf("Running UPower integration test...\n");
    
    struct cula_context *ctx = cula_create_context();
    assert(ctx != NULL);

    int res = cula_run_context(ctx);
    assert(res == 0);

    struct cula_upower *upower = cula_get_or_create_upower(ctx);
    printf("Value of on battery: %d\n", upower->data.on_battery);
    printf("Value of persentage: %f\n", upower->data.percentage);

    cula_destroy_context(ctx);
    printf("Test passed successfully!\n");
    return 0;
}
