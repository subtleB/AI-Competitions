package info.stochastic;

import java.util.*;

class Player {

    private static final int INF = Integer.MAX_VALUE / 3;
    private static final int MAX_SAMPLES = 3;
    private static final int MAX_MOLECULES = 10;

    private static class Molecule {
        String type;
        int count;

        public Molecule(String type, int count) {
            this.type = type;
            this.count = count;
        }

        @Override
        public String toString() {
            return "(" + type + "," + count + ")";
        }
    };

    /**************************************************************/
    /**************************************************************/
    /********************** Storage class *************************/

    private static class Storage {
        private Molecule a = new Molecule("A", 0);
        private Molecule b = new Molecule("B", 0);
        private Molecule c = new Molecule("C", 0);
        private Molecule d = new Molecule("D", 0);
        private Molecule e = new Molecule("E", 0);

        public Storage() {}

        public Storage(int a, int b, int c, int d, int e) {
            update(a, b, c, d, e);
        }

        public void update(int a, int b, int c, int d, int e) {
            this.a.count = a;
            this.b.count = b;
            this.c.count = c;
            this.d.count = d;
            this.e.count = e;
        }

        public int getA() {return a.count;}
        public int getB() {return b.count;}
        public int getC() {return c.count;}
        public int getD() {return d.count;}
        public int getE() {return e.count;}

        public Storage copy() {
            return new Storage(a.count, b.count, c.count,
                    d.count, e.count);
        }

        public int moleculesCount() {
            return a.count + b.count + c.count + d.count + e.count;
        }

        public Storage add(Storage storage) {
            return new Storage(
                    a.count + storage.a.count,
                    b.count + storage.b.count,
                    c.count + storage.c.count,
                    d.count + storage.d.count,
                    e.count + storage.e.count
            );
        }

        public Storage subtract(Storage storage) {
            return new Storage(
                    a.count - storage.a.count,
                    b.count - storage.b.count,
                    c.count - storage.c.count,
                    d.count - storage.d.count,
                    e.count - storage.e.count
            );
        }

        public Storage subtractOrZero(Storage storage) {
            return new Storage(
                    Math.max(0, a.count - storage.a.count),
                    Math.max(0, b.count - storage.b.count),
                    Math.max(0, c.count - storage.c.count),
                    Math.max(0, d.count - storage.d.count),
                    Math.max(0, e.count - storage.e.count)
            );
        }


        public void moleculeAdd(String molecule) {
            switch(molecule) {
                case "A" : a.count++; break;
                case "B" : b.count++; break;
                case "C" : c.count++; break;
                case "D" : d.count++; break;
                case "E" : e.count++; break;
                default : break;
            }
        }

        public void moleculeDel(String molecule) {
            switch(molecule) {
                case "A" : a.count--; break;
                case "B" : b.count--; break;
                case "C" : c.count--; break;
                case "D" : d.count--; break;
                case "E" : e.count--; break;
                default : break;
            }
        }

        @Override
        public String toString() {
            return "[count = " + moleculesCount() + "; " + a + ","
                    + b + "," + c + "," + d + "," + e + "]";
        }

        public boolean moreOrEqual(Storage storage) {
            return  (a.count >= storage.a.count)
                    && (b.count >= storage.b.count)
                    && (c.count >= storage.c.count)
                    && (d.count >= storage.d.count)
                    && (e.count >= storage.e.count);
        }

        public String getNext(Storage storage) {
            if (a.count < storage.a.count) {
                return a.type;
            } else if (b.count < storage.b.count) {
                return b.type;
            } else if (c.count < storage.c.count) {
                return c.type;
            } else if (d.count < storage.d.count) {
                return d.type;
            } else {
                return e.type;
            }
        }

        public String getMinExpertise() {
            String result = "A";
            int minCount = a.count;

            if (b.count < minCount) {
                minCount = b.count;
                result = "B";
            }

            if (c.count < minCount) {
                minCount = c.count;
                result = "C";
            }

            if (d.count < minCount) {
                minCount = d.count;
                result = "D";
            }

            if (e.count < minCount) {
                result = "E";
            }

            return result;
        }
    }

    /**************************************************************/
    /**************************************************************/
    /********************** Sample class **************************/

    private static class Sample {
        public final int id;
        public final int carriedBy;
        public final int rank;
        public final int health;
        public final String expertiseGain;
        private Storage cost = new Storage();

        public Sample(int id, int carriedBy, int rank,
                      int health, String expertiseGain, Storage cost) {
            this.id = id;
            this.carriedBy = carriedBy;
            this.rank = rank;
            this.health = health;
            this.expertiseGain = expertiseGain;
            this.cost = cost;
        }

        public Storage getCost() {
            return cost;
        }

        @Override
        public String toString() {
            return "(id:" + id + ", carriedBy:" + carriedBy + ", rank:"
                    + rank + ", expertGain:" + expertiseGain + ", health:"
                    + health + ", molecules:" + cost + ")";
        }

        @Override
        public boolean equals(Object obj) {

            if (null == obj) {
                return false;
            }

            if (!(obj instanceof Sample)) {
                return false;
            }

            Sample object = (Sample)obj;

            return id == object.id;
        }

        @Override
        public int hashCode() {
            return id;
        }

    }

    /**************************************************************/
    /**************************************************************/
    /********************** Robot class ***************************/

    private static class Robot {
        private Storage molecules = new Storage();
        private Storage expertise = new Storage();
        private ArrayList<Sample> samples = new ArrayList<>();
        private String target;
        private int eta;
        private int score;
        private int deletedMolecules;

        public void update(String target, int eta, int score,
                           Storage molecules, Storage expertise) {
            this.target = target;
            this.eta = eta;
            this.score = score;
            this.molecules = molecules;
            this.expertise = expertise;
            this.samples.clear();
            this.deletedMolecules = 0;
        }

        public Storage getMolecules() {
            return molecules;
        }

        public Storage getExpertise() {
            return expertise;
        }

        public ArrayList<Sample> getSamples() {
            return samples;
        }

        public String getTarget() {
            return target;
        }

        public int getDeletedMolecules() {
            return deletedMolecules;
        }

        public void addSample(Sample sample) {
            samples.add(sample);
        }

        public boolean haveEnoughMolecules(Sample sample) {
            return haveEnoughMolecules(sample, expertise);
        }

        public boolean haveEnoughMolecules(Sample sample, Storage newExpertise) {
            return molecules.add(newExpertise).moreOrEqual(sample.getCost());
        }

        public String getNextMolecule(Sample sample) {
            return molecules.add(expertise).getNext(sample.getCost());
        }

        public String getNextRandomMolecule(Storage available, Storage oppExpertise) {

            String  best = oppExpertise.getMinExpertise();
            if (available.moreOrEqual(new Storage(1,0,0,0,0)) && "A".equals(best)) return "A";
            if (available.moreOrEqual(new Storage(0,1,0,0,0)) && "B".equals(best)) return "B";
            if (available.moreOrEqual(new Storage(0,0,1,0,0)) && "C".equals(best)) return "C";
            if (available.moreOrEqual(new Storage(0,0,0,1,0)) && "D".equals(best)) return "D";
            if (available.moreOrEqual(new Storage(0,0,0,0,1)) && "E".equals(best)) return "E";

            if (available.moreOrEqual(new Storage(1,0,0,0,0))) return "A";
            if (available.moreOrEqual(new Storage(0,1,0,0,0))) return "B";
            if (available.moreOrEqual(new Storage(0,0,1,0,0))) return "C";
            if (available.moreOrEqual(new Storage(0,0,0,1,0))) return "D";
            if (available.moreOrEqual(new Storage(0,0,0,0,1))) return "E";

            return "Err";
        }

        public String tryToHamperTheOpp(Robot opp, Storage available) {
            String result = null;
            int minToGet = INF;
            for (Sample sample : opp.getSamples()) {
                Storage availableCopy = available.copy();
                Storage need = sample.getCost().subtractOrZero(opp.getExpertise())
                        .subtractOrZero(opp.getMolecules());

                debug(sample + " need = " + need);

                String curResult = null;
                int curMin = INF;
                if (availableCopy.moreOrEqual(need)) {
                    if (need.getA() > 0 && availableCopy.getA() - need.getA() < curMin) {
                        curResult = "A";
                        curMin = availableCopy.getA() - need.getA();
                    }

                    if (need.getB() > 0 && availableCopy.getB() - need.getB() < curMin) {
                        curResult = "B";
                        curMin = availableCopy.getB() - need.getB();
                    }

                    if (need.getC() > 0 && availableCopy.getC() - need.getC() < curMin) {
                        curResult = "C";
                        curMin = availableCopy.getC() - need.getC();
                    }

                    if (need.getD() > 0 && availableCopy.getD() - need.getD() < curMin) {
                        curResult = "D";
                        curMin = availableCopy.getD() - need.getD();
                    }

                    if (need.getE() > 0 && availableCopy.getE() - need.getE() < curMin) {
                        curResult = "E";
                        curMin = availableCopy.getE() - need.getE();
                    }
                }

                if (curMin < 3 && curMin < minToGet) {
                    minToGet = curMin;
                    result = curResult;
                }
            }

            return result;
        }

        public Sample nextUndiagnosted() {
            for (Sample sample : samples) {
                if (sample.health == -1) {
                    return sample;
                }
            }
            return null;
        }

        public boolean hasUndiagnosted() {
            for (Sample sample : samples) {
                if (sample.health == -1) {
                    return true;
                }
            }
            return false;
        }

        public boolean canProduceAnySample(Storage molecules) {
            for (Sample sample : samples) {
                if (this.molecules
                        .add(molecules)
                        .add(expertise)
                        .moreOrEqual(sample.getCost())) {
                    return true;
                }
            }
            return false;
        }

        public Sample cantProduceSample(Storage molecules) {
            for (Sample sample : samples) {
                if (!this.molecules
                        .add(molecules)
                        .add(expertise)
                        .moreOrEqual(sample.getCost())) {
                    return sample;
                }
            }
            return null;
        }

        // NOTE: Time consuming method
        public Sample nextSampleToProduce(Storage available) {
            Sample sample = null;
            int minValue = INF;

            for (Sample curSample : samples) {
                Storage newExpertise = expertise.copy();
                ArrayList<Sample> newSamples = new ArrayList<>(samples);
                newSamples.remove(curSample);

                /*debug(
                        "***************** \n"
                                + "expertise = " + expertise + "\n"
                                + "curSample = " + curSample + "\n"
                                + "available = " + available + "\n"
                                + "canProduce = " + canProduceSample(available, curSample, expertise) + "\n"
                                + "*****************"
                ); */

                int value = 0;
                if (canProduceSample(available, curSample, expertise)) {
                    newExpertise.moleculeAdd(curSample.expertiseGain);
                    int curSampleCost = curSample.getCost()
                            .subtractOrZero(newExpertise).moleculesCount();
                    value = nextSampleToProduce(newSamples, available, newExpertise, 0) + curSampleCost;
                }

                if (value > 0 && value < minValue) {
                    minValue = value;
                    sample = curSample;
                }
            }

            return sample;
        }

        private int nextSampleToProduce(ArrayList<Sample> newSamples,
                                        Storage available, Storage newExpertise, int skipped) {
            if (newSamples.size() == 0) {
                return skipped * 10;
            }

            int minValue = INF;
            for (Sample curSample : newSamples) {
                Storage curExpertise = newExpertise.copy();
                ArrayList<Sample> nextSamples = new ArrayList<>(newSamples);
                nextSamples.remove(curSample);

                int value = 0;
                if (canProduceSample(available, curSample, newExpertise)) {
                    curExpertise.moleculeAdd(curSample.expertiseGain);
                    int curSampleCost = curSample.getCost()
                            .subtractOrZero(curExpertise).moleculesCount();
                    value = nextSampleToProduce(nextSamples, available, curExpertise, skipped) + curSampleCost;
                }

                if (value > 0 && value < minValue) {
                    minValue = value;
                }
            }

            return minValue == INF ? 0 : minValue;
        }

        // Returns list of Sample with the samples we can produce using molecules we have
        public ArrayList<Sample> getSamplesWeCanProduce(ArrayList<Sample> newSamples,
                                                        ArrayList<Sample> toDelete, Storage newExpertise,
                                                        Storage myMolecules, int skipped) {
            if (skipped + toDelete.size() == samples.size()) {
                return toDelete;
            }

            int maxValue = 0;
            ArrayList<Sample> result = null;
            for (Sample curSample : newSamples) {
                if (curSample.health == -1) {
                    continue;
                }

                Storage curExpertise = newExpertise.copy();
                Storage newMolecules = myMolecules.copy();
                ArrayList<Sample> nextSamples = new ArrayList<>(newSamples);
                ArrayList<Sample> nextToDelete = new ArrayList<>(toDelete);

                nextSamples.remove(curSample);

                /*debug(
                      "***************** \n"
                    + "myMolecules = " + myMolecules + "\n"
                    + "curExpertise = " + curExpertise + "\n"
                    + "Cost = " + curSample.getCost()
                    + "*****************"
                ); */

                if (newMolecules.add(curExpertise).moreOrEqual(curSample.getCost())) {
                    nextToDelete.add(curSample);
                    curExpertise.moleculeAdd(curSample.expertiseGain);
                    newMolecules = newMolecules.subtract(curSample.getCost().subtractOrZero(curExpertise));
                    nextToDelete = getSamplesWeCanProduce(nextSamples, nextToDelete,
                            curExpertise, newMolecules, skipped);
                } else {
                    nextToDelete = getSamplesWeCanProduce(nextSamples, nextToDelete,
                            curExpertise, newMolecules, skipped + 1);
                }

                if (null != nextToDelete && nextToDelete.size() > maxValue) {
                    result = nextToDelete;
                    maxValue = nextToDelete.size();
                }
            }

            return result;
        }

        public void deleteSamplesWeCanProduce(ArrayList<Sample> toDelete) {
            if (null == toDelete) {
                return;
            }

            for (Sample sample : toDelete) {
                samples.remove(sample);
                deletedMolecules += sample.getCost().subtractOrZero(expertise).moleculesCount();
                molecules = molecules.subtract(sample.getCost().subtractOrZero(expertise));
                expertise.moleculeAdd(sample.expertiseGain);
            }
        }

        public boolean canProduceSample(Storage available, Sample sample) {
            return canProduceSample(available, sample, expertise);
        }

        public boolean canProduceSample(Storage available, Sample sample, Storage newExpertise) {
            return this.molecules
                    .add(available)
                    .add(newExpertise)
                    .moreOrEqual(sample.getCost());
        }

        @Override
        public String toString() {
            String samplesToString = "\n";

            for (Sample sample : samples) {
                samplesToString += "\t\t" + sample + "\n";
            }

            return "(target:" + target + ", samples:" + samples.size()
                    + ", eta:" + eta + ", score:"
                    + score + ", molecules:" + molecules
                    + ", expertise:" + expertise + ")" + samplesToString;
        }
    }

    /**************************************************************/
    /**************************************************************/
    /**************************************************************/

    private interface IStrategy {
        String makeMove();
    }

    private static class DecisionMaker {
        private IStrategy strategy;

        public DecisionMaker(IStrategy strategy) {
            this.strategy = strategy;
        }

        public String getNextMove() {
            return strategy.makeMove();
        }
    }

    /**************************************************************/
    /**************************************************************/
    /**************************************************************/
    /************** Strategy implementation below *****************/

    private static class Strategy implements IStrategy {

        Robot me;
        Robot opp;
        Storage available;
        ArrayList<Sample> samples;

        Strategy(Robot me, Robot opp,
                 Storage available, ArrayList<Sample> samples) {
            this.me = me;
            this.opp = opp;
            this.available = available;
            this.samples = samples;
        }

        private String hamper(String canHamper) {
            if (null != canHamper && me.getMolecules().moleculesCount()
                    + me.getDeletedMolecules() < MAX_MOLECULES) {
                return "CONNECT " + canHamper;
            }
            return "GOTO LABORATORY";
        }

        @Override
        public String makeMove() {

            /****************************************/
            /**************** START *****************/
            /****************************************/
            if ("START_POS".equals(me.getTarget())) {
                return "GOTO SAMPLES";
            }

            /****************************************/
            /*************** SAMPLES ****************/
            /****************************************/
            if ("SAMPLES".equals(me.getTarget())) {
                if (me.getSamples().size() < MAX_SAMPLES) {
                    if (me.getExpertise().moleculesCount() < 12) {
                        return "CONNECT 1";
                    } else {
                        return "CONNECT 3";
                    }
                }
                return "GOTO DIAGNOSIS";
            }

            /****************************************/
            /************** DIAGNOSIS ***************/
            /****************************************/
            if ("DIAGNOSIS".equals(me.getTarget())) {
                ArrayList<Sample> toDelete = me.getSamplesWeCanProduce(
                        me.getSamples(),
                        new ArrayList<>(),
                        me.getExpertise(), me.getMolecules(), 0);

                int canProduce = null == toDelete ? 0 : toDelete.size();

                // Can produce without additional molecules
                debug("canProduce = " + canProduce);
                if (canProduce == 3) {
                    return "GOTO LABORATORY";
                }

                if (me.getSamples().size() > 0 && me.hasUndiagnosted()) {
                    Sample sample = me.nextUndiagnosted();
                    return "CONNECT " + sample.id;
                } else {

                    if (me.getMolecules().moleculesCount() == MAX_MOLECULES) {
                        Sample badSample = me.cantProduceSample(available);
                        if (badSample != null) {
                            return "CONNECT " + badSample.id;
                        }
                    }

                    if (samples.size() > 0 && me.getSamples().size() < MAX_SAMPLES) {
                        Sample sample = samples.get(0);
                        if (me.canProduceSample(available, sample)) {
                            samples.remove(0);
                            return "CONNECT " + sample.id;
                        }
                    }

                    // Number of diagnosted samples
                    if (me.getSamples().size() > 0) {
                        return "GOTO MOLECULES";
                    } else {
                        return "GOTO SAMPLES";
                    }
                }
            }

            /****************************************/
            /************** MOLECULES ***************/
            /****************************************/
            if ("MOLECULES".equals(me.getTarget())) {
                String canHamper = me.tryToHamperTheOpp(opp,available);
                ArrayList<Sample> toDelete = me.getSamplesWeCanProduce(
                        me.getSamples(),
                        new ArrayList<>(),
                        me.getExpertise(), me.getMolecules(), 0);
                me.deleteSamplesWeCanProduce(toDelete);

                int canProduce = null == toDelete ? 0 : toDelete.size();
                if (canProduce == 3 || (canProduce > 0 && me.getSamples().size() == 0)) {
                    return hamper(canHamper);
                }

                if (me.getSamples().size() > 0 && (me.canProduceAnySample(available) || canProduce > 0)) {
                    Sample sample = me.nextSampleToProduce(available);
                    if (null == sample
                            || (!me.canProduceSample(available, sample, me.getExpertise()))) {
                        return hamper(canHamper);
                    }

                    if (!me.haveEnoughMolecules(sample)) {
                        if (me.getMolecules().moleculesCount() + me.getDeletedMolecules() < MAX_MOLECULES) {
                            return "CONNECT " + me.getNextMolecule(sample);
                        } else {
                            return canProduce > 0 ? "GOTO LABORATORY" : "GOTO DIAGNOSIS";
                        }
                    }

                } else {
                    return "GOTO DIAGNOSIS";
                }
                return hamper(canHamper);
            }

            /****************************************/
            /************** LABORATORY **************/
            /****************************************/
            if ("LABORATORY".equals(me.getTarget())) {
                for (Sample sample : me.getSamples()) {
                    if (me.haveEnoughMolecules(sample)) {
                        return "CONNECT " + sample.id;
                    }
                }
                return me.getSamples().size() > 1 ? "GOTO MOLECULES" : "GOTO SAMPLES";
            }

            return "ERROR";
        }
    }

    /**************************************************************/
    /**************************************************************/
    /**************************************************************/

    private static void debugInput(Robot me, Robot opp,
                                   Storage available, ArrayList<Sample> samples) {
        System.err.println(available);
        System.err.println(me);
        System.err.println(opp);
        for(Sample sample : samples) {
            System.err.println(sample);
        }
    }

    private static void debug(String msg) {
        System.err.println(msg);
    }

    public static void main(String args[]) {
        Scanner in = new Scanner(System.in);

        Robot player1 = new Robot();
        Robot player2 = new Robot();
        Storage available = new Storage(INF, INF, INF, INF, INF);
        ArrayList<Sample> samples = new ArrayList<>();
        Strategy strategy = new Strategy(player1, player2, available, samples);
        DecisionMaker decisionMaker = new DecisionMaker(strategy);

        int projectCount = in.nextInt();
        for (int i = 0; i < projectCount; i++) {
            Storage project = new Storage(in.nextInt(), in.nextInt(),
                    in.nextInt(), in.nextInt(), in.nextInt());
            System.err.println(project);
        }

        // Game loop
        while (true) {
            player1.update(
                    in.next(), in.nextInt(), in.nextInt(),
                    new Storage(in.nextInt(), in.nextInt(),
                            in.nextInt(), in.nextInt(), in.nextInt()),
                    new Storage(in.nextInt(), in.nextInt(),
                            in.nextInt(), in.nextInt(), in.nextInt())
            );

            player2.update(
                    in.next(), in.nextInt(), in.nextInt(),
                    new Storage(in.nextInt(), in.nextInt(),
                            in.nextInt(), in.nextInt(), in.nextInt()),
                    new Storage(in.nextInt(), in.nextInt(),
                            in.nextInt(), in.nextInt(), in.nextInt())
            );

            available.update(in.nextInt(), in.nextInt(),
                    in.nextInt(), in.nextInt(), in.nextInt());

            samples.clear();
            int sampleCount = in.nextInt();
            for (int i = 0; i < sampleCount; i++) {
                int sampleId = in.nextInt();
                int carriedBy = in.nextInt();
                int rank = in.nextInt();
                String expertiseGain = in.next();
                int health = in.nextInt();
                int costA = in.nextInt();
                int costB = in.nextInt();
                int costC = in.nextInt();
                int costD = in.nextInt();
                int costE = in.nextInt();

                Sample sample = new Sample(sampleId, carriedBy, rank,
                        health, expertiseGain,
                        new Storage(costA, costB, costC, costD, costE)
                );

                if (sample.carriedBy == 0) {
                    player1.addSample(sample);
                } else if (sample.carriedBy == 1) {
                    player2.addSample(sample);
                } else {
                    samples.add(sample);
                }
            }

            System.out.println(decisionMaker.getNextMove());
        }
    }
}