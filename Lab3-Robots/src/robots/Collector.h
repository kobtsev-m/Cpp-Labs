#ifndef LAB3_ROBOTS_COLLECTOR_H
#define LAB3_ROBOTS_COLLECTOR_H

#include "AbstractRobot.h"

class Collector : public AbstractRobot {
private:
    int apples;
public:
    explicit Collector(int id);

    std::pair<cell_t, cell_t> getOwnType() override;
    cell_t getItemType() override;
    std::vector<cell_t> getGoodCells() override;
    int getItemsCount() const override;
    void onGrab() override;
};


#endif //LAB3_ROBOTS_COLLECTOR_H
