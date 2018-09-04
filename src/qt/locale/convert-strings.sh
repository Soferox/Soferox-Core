#!/bin/bash

sed -i "s/\bbitcoin\b/soferox/g" *.ts
sed -i "s/\bBitcoin\b/Soferox/g" *.ts
sed -i "s/\bBITCOINS\b/SOFEROXS/g" *.ts
sed -i "s/\bbitcoins\b/soferoxs/g" *.ts
sed -i "s/\bBitcoins/Soferoxs/g" *.ts

sed -i "s/ bitcoin/ soferox/g" *.ts
sed -i "s/ Bitcoin/ Soferox/g" *.ts
sed -i "s/ BITCOIN/ SOFEROX/g" *.ts
