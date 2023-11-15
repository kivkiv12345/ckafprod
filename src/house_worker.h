typedef struct house_data_s {
    unsigned int id;
    unsigned int num_adults;
    unsigned int num_children;
    unsigned int house_size_m2;
    unsigned int no_electric_cars;
} house_data_t;

void stop_house_simulations(void);

/**
 * @brief Continuously simulates data for the provided house
 * 
 * @param house_data House data for type house_data_t*
 */
void *houseworker_thread(void *house_data);
