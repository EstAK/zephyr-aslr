from typing import List, Tuple
from sys import argv

class Entry:
    def __init__(self, filename: str, csv: bool = False):
        self.filename = filename
        self.aslr: bool = False
        self.content: List[List[str]] = self._read_file(filename)
        self.folded_content: List[Tuple[str, str, int, int]] = list()
        self._pairwise_fold()

        if csv :
            self._write_csv()

        self.avg = sum([ a[3] - a[2] for a in self.folded_content]) / len(self.folded_content)
        print(f"{filename} : (ASLR {"ON" if self.aslr else "OFF"}avg time difference -> {"%.2f" % self.avg} cycles")

    def _write_csv(self) -> None:
        with open(self.filename.replace("txt", "csv"), "w") as f:
            f.write("from,to,time(cycles)\n")
            for entry in self.folded_content:
                f.write(f"{",".join(entry[:2])}, {str(entry[3] - entry[2])}")
                f.write("\n")


    def _pairwise_fold(self) -> None:
        for i in range(0, len(self.content), 2):
            if i + 1 < len(self.content):
                self.folded_content.append((self.content[i][0], self.content[i + 1][0], int( self.content[i][1]),
                                        int(self.content[i + 1][1])))
        return None;

    def _read_file(self, filename :str) -> List[List[str]]:
        res : List[List[str]] = list()
        first_yield : bool = True

        with open(filename, "r") as f:
            temp: List[str] = list()
            splitted: List[str] = list()
            self.aslr = f.readline().startswith("ASLR enabled")
            for line in f.readlines()[1:]:
                splitted = line.split(" ")
                temp.append(splitted.pop(0))
                temp.append(splitted.pop(-1).removesuffix("\n"))
                match " ".join(splitted):
                    case "took over at":
                        pass
                    case "yields at" | "is back" :
                        if first_yield:
                            first_yield = False
                            pass
                        res.append(temp)
                temp = list()
                splitted = list()
        return res

def read_file(filename: str) -> None:
    return None

if __name__ == "__main__":
    csv: bool = not argv[1] == "-no-csv"
    print(csv)
    for file in argv[1 if csv else 2 :]:
        Entry(file, csv)
