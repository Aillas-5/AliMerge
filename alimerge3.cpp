// This program is designed to look for merges of Aliquot
// sequences.  It uses the file OE_3000000_C80.txt to
// match the last occurrence of an 80 digit composite and
// determines the merged sequence, if any.  If a merge is
// found, the program determines the merge points between
// the two sequences.  This program is specifically built
// to work with base tables found at:
//    "Aliquot sequences starting on integer powers n^i"
// (http://www.aliquotes.com/aliquotes_puissances_entieres.html)
//
// It can be compiled in linux via:
//           g++ <sourcename> -o <finalname>
/////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>

using namespace std;

int main (int argc, char **argv) {

    ifstream C80, ali1, ali2;
    string C80list[50000], ali1list[20000], ali2list[20000];
    string buff1, buff2, buff3, buff4, buffali1, buffali2, getfile;
    int C80i=0, ali1i, ali2i, base, first, last, exp;
    int i, j, k;
    size_t found, foundp, founde, founds;

    if(argc<4){
        cout << "Please invoke as: <./program> <base> <starting exponent> <ending exponent>" << endl;
        return 0;
    }

    sscanf(argv[1],"%d",&base);
    sscanf(argv[2],"%d",&first);
    sscanf(argv[3],"%d",&last);

    C80.open("OE_3000000_C80.txt");
    if(C80.is_open()){
        while(!C80.eof()){
            getline(C80, C80list[C80i++]);
        }
        C80.close();
    }
    else {
        cout << "The 80 digit file was not found - download it? (y/n): ";
        cin >> getfile;
        if(getfile=="y")
            system("wget \"http://www.aliquotes.com/OE_3000000_C80.txt\" -q -O OE_3000000_C80.txt");
        C80i=0;
        C80.open("OE_3000000_C80.txt");
        if(C80.is_open()){
            while(!C80.eof()){
                getline(C80, C80list[C80i++]);
            }
            C80.close();
        }
        else{
            cout << "Trouble has occurred while trying to read the 80 digit file!" << endl;
            return 0;
        }
    }

    cout << "Running base " << base << " from " << first << " through " << last << " . . ." << endl;

    for(exp=first;exp<=last;exp++){
        buffali1.assign("wget \"http://www.factordb.com/elf.php?seq=");
        buffali1.append(to_string(base));
        buffali1.append("^");
        buffali1.append(to_string(exp));
        buffali1.append("&type=1\" -q -O aliseq1");
        system(buffali1.c_str());

        ali1i=0;
        ali1.open("aliseq1");
        if(ali1.is_open()){
            while(!ali1.eof()){
                getline(ali1,ali1list[ali1i++]);
            }
            ali1.close();
        }
        else{
            cout << "aliseq1 was not read properly!" << endl;
            return 0;
        }

        buff2.assign("");
        for(i=0;i<ali1i;i++){
            foundp=ali1list[i].find(".");
            founde=ali1list[i].find("=");
            if(founde-foundp==85){
                buff2.assign(ali1list[i].substr(foundp+4,80));
            }
        }
        if(buff2.length()==80){
            for(i=0;i<C80i;i++){
                found=C80list[i].find(buff2);
                if(found!=string::npos){
                    founds=C80list[i].find(" ",2);
                    buffali2.assign("wget \"http://www.factordb.com/elf.php?seq=");
                    buffali2.append(C80list[i].substr(1,founds-1));
                    buffali2.append("&type=1\" -q -O aliseq2");
                    system(buffali2.c_str());

                    ali2i=0;
                    ali2.open("aliseq2");
                    if(ali2.is_open()){
                        while(!ali2.eof()){
                            getline(ali2,ali2list[ali2i++]);
                        }
                        ali2.close();
                    }
                    else{
                        cout << "aliseq2 was not read properly!" << endl;
                        return 0;
                    }

                    for(j=0;j<ali1i;j++){
                        foundp=ali1list[j].find(".");
                        buff3=ali1list[j].substr(foundp);
                        for(k=0;k<ali2i;k++){
                            found=ali2list[k].find(buff3);
                            if(found!=string::npos){
                                buff4.assign(to_string(base));
                                buff4.append("^");
                                buff4.append(to_string(exp));
                                buff4.append(":i");
                                buff4.append(ali1list[j].substr(0,foundp-1));
                                buff4.append(" merges with ");
                                buff4.append(C80list[i].substr(1,founds-1));
                                buff4.append(":i");
                                foundp=ali2list[k].find(".");
                                buff4.append(ali2list[k].substr(0,foundp-1));
                                cout << buff4 << endl;;
                                k=ali2i;
                                j=ali1i;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

