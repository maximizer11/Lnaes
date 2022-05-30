using Airlanes.DataSetAirlanesTableAdapters;
using System;
using System.Windows;

namespace Airlanes
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        DataSetAirlanes dataSetAirlanes;
        UsersTableAdapter usersTableAdapter;
        System.Windows.Threading.DispatcherTimer timer = new System.Windows.Threading.DispatcherTimer();
        int time = 10;
        int countries = 0;
        public MainWindow()
        {
            InitializeComponent();
            dataSetAirlanes = new DataSetAirlanes();
            usersTableAdapter = new UsersTableAdapter();
            timer.Tick += new EventHandler(timerTick);
            timer.Interval += new TimeSpan(0, 0, 1);
        }

        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void btnLogin_Click(object sender, RoutedEventArgs e)
        {
            if (username.Text != "" && password.Password != "")
            {
                int? id_user = usersTableAdapter.Login(username.Text, password.Password);
                if (id_user != null)
                {
                    if (true == usersTableAdapter.Active(username.Text, password.Password).Value)
                    {
                        if (1 == usersTableAdapter.Role(username.Text, password.Password))
                        {
                            Administrator administrator = new Administrator();
                            Close();
                            administrator.Show();
                        }
                        else
                        {
                            Users users = new Users();
                            Close();
                            users.Show();
                        }
                    }
                    else
                    {
                        errors.Text = "Ваша учетная запись была аннулирована администрацией";
                    }
                }
                else
                {
                    if (countries == 2)
                    {
                        time = 10;
                        btnLogin.IsEnabled = false;
                        timer.Start();
                        countries = 0;
                    }
                    else
                    {
                        errors.Text = "Логин или пароль неверны, попробуйте еще раз";
                        countries++;
                    }
                }
            }
            else
            {
                errors.Text = "Введите логин или пароль";
            }
        }

        private void timerTick(object sender, EventArgs e)
        {
            errors.Text = "Повторите попытку через " + time + " секунд";
            time--;
            if (time == 0)
            {
                btnLogin.IsEnabled = true;
                errors.Text = "";
                timer.Stop();
            }
        }
    }
}
